#include "saving.h"
#include<nlohmann/json.hpp>
#include<fstream>
#include<array>
#include"deserializer.h"
#include"serializer.h"
using namespace nlohmann;
using value_t = nlohmann::detail::value_t;
using namespace DirectX;

extern bool validate(nlohmann::json& file);
extern GEOMETRY_TYPE getGeometryType(const std::string& v);

json make_point(XMFLOAT3 p)
{
	return json::object({
		std::pair("x", p.x),
		std::pair("y", p.z),
		std::pair("z", p.y)
		});
}
json make_point(XMINT2 p)
{
	return json::object({
		std::pair("x", (unsigned)p.x),
		std::pair("y", (unsigned)p.y)
		});
}
json make_tfpoint(XMINT2 p)
{
	return json::object({
		std::pair("u", (bool)p.x),
		std::pair("v", (bool)p.y)
		});
}
json make_reflist(
	std::map<Entity*, unsigned>& ids,
	std::vector<Entity*>::iterator begin,
	std::vector<Entity*>::iterator end)
{
	json l = json::array({});
	for (auto it = begin; it < end; it++)
		l.push_back(json::object({
			std::pair("id", ids[*it])
			}));
	return l;
}
json make_patchlist(
	std::map<Entity*, unsigned>& ids,
	bool deBoorMode,
	std::vector<Entity*>& controlPoints,
	std::vector<std::string_view>& names,
	unsigned& id, int samples
)
{
	json l = json::array({});
	for (int i = 0; i < controlPoints.size() / 16; i++)
	{
		l.push_back(json::object({
			std::pair("objectType",
			deBoorMode? "bezierPatchC2": "bezierPatchC0"),
			std::pair("id", ++id),
			std::pair("name", (std::string)names[i]),
			std::pair("samples", make_point({samples,samples})),
			std::pair("controlPoints",
			make_reflist(ids, controlPoints.begin() + i*16, controlPoints.begin() + i*16 + 16))
			}));
	}
	return l;
}

SaveResult save(const CadApplication& app, const char* filepath)
{
	std::ofstream f(filepath);
	if (!f.is_open())
		return SaveResult::NoFile;

	json geometry = json::array({});
	json points = json::array({});
	unsigned id = 0;
	std::map<Entity*, unsigned> ids;
	{
		auto& pts = Catalogue<PointTransform>::Instance.GetAll();
		for (int i = 0; i < pts.size(); i++)
		{
			points.push_back(json::object({
				std::pair("id", ++id),
				std::pair("name", (std::string)pts[i]->owner.GetName()),
				std::pair("position", make_point(pts[i]->Position()))
				}));
			ids[&(pts[i]->owner)] = id;
		}
	}
	{
		auto& trs = Catalogue<TorusGenerator>::Instance.GetAll();
		for (int i = 0; i < trs.size(); i++)
		{
			std::string name;
			DirectX::XMFLOAT2 radii;
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 rotation;
			DirectX::XMFLOAT3 scale;
			DirectX::XMINT2 samples;
			Serializer::Torus(&(trs[i]->owner), name,
				radii, position, rotation, scale,samples);
			geometry.push_back(json::object({
				std::pair("objectType", "torus"),
				std::pair("id", ++id),
				std::pair("name", name),
				std::pair("smallRadius", radii.x),
				std::pair("largeRadius", radii.y),
				std::pair("position", make_point(position)),
				std::pair("rotation", make_point(rotation)),
				std::pair("scale", make_point(scale)),
				std::pair("samples", make_point(samples))
				}));
			ids[&(trs[i]->owner)] = id;
		}
	}
	{
		auto& bzs = Catalogue<BezierCurve>::Instance.GetAll();
		for (int i = 0; i < bzs.size(); i++)
		{
			Entity* e = &bzs[i]->owner;
			std::string name;
			Entity::Selection controlPoints;

			Serializer::Curve(e, name, controlPoints);
			geometry.push_back(json::object({
				std::pair("objectType", 
				e->GetComponent<BezierInterpolator>()?"interpolatedC2":
				e->GetComponent<SplineGenerator>()?"bezierC2":
				"bezierC0"),
				std::pair("id", ++id),
				std::pair("name", name),
				std::pair(
					e->GetComponent<SplineGenerator>()?"deBoorPoints":
				"controlPoints", make_reflist(ids,
					controlPoints.begin(), controlPoints.end()))
				}));
			ids[e] = id;
		}
	}
	{
		auto& bss = Catalogue<BicubicSurface>::Instance.GetAll();
		for (int i = 0; i < bss.size(); i++)
		{
			Entity* e = &bss[i]->owner;
			std::string name;
			std::vector<std::string_view> segmentNames;
			Entity::Selection controlPoints;
			DirectX::XMINT2 parameterWrapped;
			DirectX::XMINT2 size;
			Serializer::Surface(e, name,
				segmentNames, controlPoints, parameterWrapped, size);
			auto segs = bss[i]->GetSegments();
			geometry.push_back(json::object({
				std::pair("objectType",
				segs[0]->deBoorMode ? "bezierSurfaceC2" :
				"bezierSurfaceC0"),
				std::pair("name", name),
				std::pair("id", ++id),
				std::pair("patches", make_patchlist(ids,
					segs[0]->deBoorMode,
					controlPoints,
					segmentNames,
					id,
					bss[i]->surfDetailOffset+4
					)),
				std::pair("parameterWrapped", make_tfpoint(bss[i]->wrapMode)),
				std::pair("size", make_point(bss[i]->division))
				}));
			ids[e] = id;
		}
	}

	json file = json::object({
		std::pair("geometry", geometry),
		std::pair("points", points)
	});

	f << file.dump(1);
	return SaveResult::Success;
}

SaveResult load(CadApplication& app, const char* filepath)
{
	std::ifstream f(filepath);
	if (!f.is_open())
		return SaveResult::NoFile;
	
	json file = json::parse(f);
	try
	{
		validate(file);
	}
	catch (nlohmann::detail::other_error e)
	{
		return SaveResult::InvalidFile;
	}
	Entity::Clear();
	app.mainFolder = Entity::New()->AddComponent<Folder>();

	json& points = file["points"];
	std::map<unsigned, Entity*> ids;
	for (auto it = points.begin(); it < points.end(); it++)
	{
		json& point = *it;
		Entity* e = Deserializer::Point(point["name"],
			XMFLOAT3{
				point["position"]["x"],
				point["position"]["z"],
				point["position"]["y"]
			}
		);
		app.mainFolder->Add(e);
		ids[point["id"]] = e;
	}
	json& geometry = file["geometry"];

	for (auto it = geometry.begin(); it < geometry.end(); it++)
	{
		json& object = *it;
		Entity* e = nullptr;
		auto geometryType = getGeometryType(object["objectType"]);
		Entity::Selection addedSegments;
		switch (geometryType)
		{
		case GEOMETRY_TYPE::torus:
			e = Deserializer::Torus(object["name"],
				XMFLOAT2{
					object["smallRadius"],
					object["largeRadius"]
				},
				XMFLOAT3{
					object["position"]["x"],
					object["position"]["z"],
					object["position"]["y"]
				},
				XMFLOAT3{
					object["rotation"]["x"],
					object["rotation"]["y"],
					object["rotation"]["z"]
				},
				XMFLOAT3{
					object["scale"]["x"],
					object["scale"]["z"],
					object["scale"]["y"]
				},
				XMINT2{
					object["samples"]["x"],
					object["samples"]["y"]
				}
			);
			{
				std::string name;
				DirectX::XMFLOAT2 radii;
				DirectX::XMFLOAT3 position;
				DirectX::XMFLOAT3 rotation;
				DirectX::XMFLOAT3 scale;
				DirectX::XMINT2 sample;
				Serializer::Torus(e, name, radii, position, rotation, scale, sample);
				int test = 0;
			}
			break;
		case GEOMETRY_TYPE::bezierC0:
		{
			Entity::Selection s;
			auto& cps = object["controlPoints"];
			for (int i = 0; i < cps.size(); i++)
				s.push_back(ids[cps[i]["id"]]);
			e = Deserializer::BezierC0(object["name"], s);
		}
			break;
		case GEOMETRY_TYPE::bezierC2:
		{
			Entity::Selection s;
			auto& cps = object["deBoorPoints"];
			for (int i = 0; i < cps.size(); i++)
				s.push_back(ids[cps[i]["id"]]);
			e = Deserializer::BezierC2(object["name"], s);
		}
			break;
		case GEOMETRY_TYPE::interpolatedC2:
		{
			Entity::Selection s;
			auto& cps = object["controlPoints"];
			for (int i = 0; i < cps.size(); i++)
				s.push_back(ids[cps[i]["id"]]);
			e = Deserializer::InterpolatedC2(object["name"], s);
		}
			break;
		case GEOMETRY_TYPE::bezierSurfaceC0:
		{
			Entity::Selection s;
			std::vector<std::string*> segmentNames;
			auto& ptchs = object["patches"];
			
			for (int i = 0; i < ptchs.size(); i++)
			{
				auto& cps = ptchs[i]["controlPoints"];
				for (int j = 0; j < cps.size(); j++)
					s.push_back(ids[cps[j]["id"]]);
				segmentNames.push_back(ptchs[i]["name"].get_ptr<std::string*>());
			}
			e = Deserializer::BezierSurfaceC0(addedSegments,
				object["name"],
				segmentNames,
				s,
				XMINT2{
					object["parameterWrapped"]["u"],
					object["parameterWrapped"]["v"]
				},
				XMINT2{
					object["size"]["x"],
					object["size"]["y"]
				}
			);
		}
			break;
		case GEOMETRY_TYPE::bezierSurfaceC2:
		{
			Entity::Selection s;
			std::vector<std::string*> segmentNames;
			auto& ptchs = object["patches"];

			for (int i = 0; i < ptchs.size(); i++)
			{
				auto& cps = ptchs[i]["controlPoints"];
				for (int j = 0; j < cps.size(); j++)
					s.push_back(ids[cps[j]["id"]]);
				segmentNames.push_back(ptchs[i]["name"].get_ptr<std::string*>());
			}
			e = Deserializer::BezierSurfaceC2(addedSegments,
				object["name"],
				segmentNames,
				s,
				XMINT2{
					object["parameterWrapped"]["u"],
					object["parameterWrapped"]["v"]
				},
				XMINT2{
					object["size"]["x"],
					object["size"]["y"]
				}
			);
		}
			break;
		case GEOMETRY_TYPE::gregoryPatch:
			break;
		case GEOMETRY_TYPE::INVALID:
			break;
		default:
			break;
		}
		if (e != nullptr)
		{
			app.mainFolder->Add(e);
			for (int i = 0; i < addedSegments.size(); i++)
				app.mainFolder->Add(addedSegments[i]);
			ids[object["id"]] = e;
		}
	}

	return SaveResult::Success;
}

SaveResult savePath(const float* path, int count, const char* filepath)
{
	std::ofstream f(filepath);
	if (!f.is_open())
		return SaveResult::NoFile;
	assert(count % 3 == 0);
	try
	{
		for (int i = 0; i < count; i += 3)
		{
			f << "N" << i / 3 + 1 << "G01";
			f << "X" << std::fixed << std::setw(5) << std::setprecision(3) << 75*(path[i + 0] * 2 - 1);
			f << "Y" << std::fixed << std::setw(5) << std::setprecision(3) << 75*(path[i + 1] * 2 - 1);
			f << "Z" << std::fixed << std::setw(5) << std::setprecision(3) << (50-16)*path[i + 2] + 16;
			f << "\n";
		}
	}
	catch (...)
	{
		return SaveResult::InvalidFile;
	}
	return SaveResult::Success;
}
