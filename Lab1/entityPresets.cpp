#include "entityPresets.h"
#include"all_components.h"
#include<string>
#include"vecmath.h"

Entity* EntityPresets::Model(DirectX::XMFLOAT3 position, Mesh* mesh)
{
	auto* e = Entity::New();
	e->AddComponent<ModelTransform>()->Translate(position);
	e->AddComponent<MeshRenderer>()->mesh = mesh;

	static unsigned modelI = 1;
	std::string name = "Mesh object " + std::to_string(modelI++);
	e->SetName(name.c_str());
	//e->Select(true);
    return e;
}

Entity* EntityPresets::Point(DirectX::XMFLOAT3 position)
{
	auto* e = Entity::New();
	auto* t = e->AddComponent<PointTransform>();
	t->Translate(position);
	e->AddComponent<PointRenderer>();

	static unsigned pointI = 1;
	std::string name = "Point " + std::to_string(pointI++);
	e->SetName(name.c_str());
	//e->Select(true);
	return e;
}

Entity* EntityPresets::BezierCurveObject(const std::vector<Entity*>& selection)
{
	if (selection.size() < 1)
		return nullptr;
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(selection);
	e->AddComponent<BezierCurve>();

	static unsigned curveI = 1;
	std::string name = "Bezier " + std::to_string(curveI++);
	e->SetName(name.c_str());
	//e->Select(true);

	return e;
}

Entity* EntityPresets::SplineCurve(const std::vector<Entity*>& selection)
{
	if (selection.size() < 4)
		return nullptr;
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(selection);
	e->AddComponent<VPointCollection>();
	e->AddComponent<SplineGenerator>();
	e->AddComponent<BezierCurve>();

	static unsigned curveI = 1;
	std::string name = "Spline " + std::to_string(curveI++);
	e->SetName(name.c_str());
	//e->Select(true);

	return e;
}

Entity* EntityPresets::InterpCurve(const std::vector<Entity*>& selection)
{
	if (selection.size() < 4)
		return nullptr;
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(selection);
	e->AddComponent<VPointCollection>()->ShowVirtualPoints(false);
	e->AddComponent<BezierInterpolator>();
	e->AddComponent<BezierCurve>();

	static unsigned curveI = 1;
	std::string name = "Interpolating Curve " + std::to_string(curveI++);
	e->SetName(name.c_str());
	//e->Select(true);

	return e;
}

Entity* EntityPresets::BicubicSurfaceObject(DirectX::XMFLOAT3 position, const BicubicSurfaceParams& params,
	std::vector<Entity*>& outPoints, std::vector<Entity*>& outSegments)
{
	int w, h;
	if (params.deBoor)
	{
		assert(!(params.wrapMode & SURFACE_WRAP_U) || params.division.x >= 3
			&& "Invalid segment count for cylinder");
		w = params.division.x + ((params.wrapMode & SURFACE_WRAP_U) ? 0 : 3);
		h = params.division.y + 3;
	}
	else
	{
		w = params.division.x * 3 + ((params.wrapMode & SURFACE_WRAP_U) ? 0 : 1);
		h = params.division.y * 3 + 1;
	}

	static unsigned surfI = 1;

	auto* surf = Entity::New();
	auto* pc = surf->AddComponent<PointCollection>();
	auto collective = surf->AddComponent<BicubicSurface>();
	pc->isMutable = false;

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			DirectX::XMFLOAT3 offset;
			if (params.wrapMode == SURFACE_WRAP_NONE)
				offset =
			{
				(x / (w - 1.0f) - 0.5f) * params.dimensions.x,
				(y / (h - 1.0f) - 0.5f) * params.dimensions.y,
				0
			};
			else
				offset =
			{
				sin(x * DirectX::XM_2PI / w) * params.dimensions.x,
				(y / (h - 1.0f) - 0.5f) * params.dimensions.y,
				cos(x * DirectX::XM_2PI / w) * params.dimensions.x
			};
			auto* e = Entity::New();
			e->AddComponent<PointTransform>()
				->Translate(position + offset);
			e->AddComponent<PointRenderer>();
			outPoints.push_back(e);
			std::string name = "Bicubic Surface " + std::to_string(surfI) +
				" Point " + std::to_string(outPoints.size());
			e->SetName(name.c_str());
		}
	pc->AddSelection(outPoints);

	for (int y = 0; y < params.division.y; y++)
		for (int x = 0; x < params.division.x; x++)
		{
			auto* e = Entity::New();
			auto* bcs = e->AddComponent<BicubicSegment>();
			for (int vi = 0; vi < 4; vi++)
			for (int ui = 0; ui < 4; ui++)
				{
				int ind;
				if (params.deBoor)
					ind = ui + x + (vi + y) * w
						- ((ui + x >= w) ? w : 0);
				else
					ind = ui + x * 3 + (vi + y * 3) * w
						- ((ui + x * 3 >= w) ? w : 0);
					bcs->indices[ui + vi * 4] = ind;
				}
			bcs->SetSource(pc);
			bcs->deBoorMode = params.deBoor;
			outSegments.push_back(e);
			std::string name = "Bicubic Surface " + std::to_string(surfI) + " Segment " + std::to_string(outSegments.size());
			e->SetName(name.c_str());
		}

	for (int x = 0; x < params.division.x; x++)
		for (int y = 0; y < params.division.y; y++)
		{
			if (y != 0)
			{
				auto* top = outSegments[x + (y-1) * params.division.x];
				auto* bot = outSegments[x + y * params.division.x];
				top->GetComponent<BicubicSegment>()->neighbors[2] = bot;
				bot->GetComponent<BicubicSegment>()->neighbors[0] = top;
			}
			if (x != 0)
			{
				auto* left = outSegments[(x-1) + y * params.division.x];
				auto* right = outSegments[x + y * params.division.x];
				left->GetComponent<BicubicSegment>()->neighbors[1] = right;
				right->GetComponent<BicubicSegment>()->neighbors[3] = left;
			}
		}
	if (params.wrapMode & SURFACE_WRAP_U)
	{
		for (int y = 0; y < params.division.y; y++)
		{
			auto* left = outSegments[params.division.x - 1 + y * params.division.x];
			auto* right = outSegments[0 + y * params.division.x];
			left->GetComponent<BicubicSegment>()->neighbors[1] = right;
			right->GetComponent<BicubicSegment>()->neighbors[3] = left;
		}
	}

	surfI++;
	collective->SetSegments(outSegments);
	std::string name = "Bicubic Surface " + std::to_string(surfI);
	surf->SetName(name.c_str());
	return surf;
}

void EntityPresets::FillInSurface(Entity::Selection s, std::vector<Entity*>& outSurfs)
{

	std::vector<BicubicSegment*> bss{};
	for (int i = 0; i < s.size(); i++)
	{
		auto* bs = s[i]->GetComponent<BicubicSegment>();
		if (bs)
		{
			bss.push_back(bs);
			if (bss.size() > 3)
				return;
		}
	}
	if (bss.size() < 3)
		return;


	for (int i = 0; i < 4; i++)
	{
		std::vector<BicubicSegment*> ordered;
		std::vector<int> sides;
		if (bss[2]->neighbors[i] != nullptr)
			continue;

		if (bss[2]->neighbors[(i + 1) % 4]->GetComponent<BicubicSegment>() == bss[1])
			ordered = { bss[2],bss[1],bss[0] };
		else if (bss[2]->neighbors[(i + 1) % 4]->GetComponent<BicubicSegment>() == bss[0])
			ordered = { bss[2],bss[0],bss[1] };
		else
			continue;

		sides = { i };

		for (int j = 1; j < 3; j++)
		{
			int n = std::find(ordered[j]->neighbors.begin(), ordered[j]->neighbors.end(), &ordered[j-1]->owner)
				- ordered[j]->neighbors.begin();
			sides.push_back((n + 4 + (bss[j]->ccwNeighbor[n] ? -1 : 1)) % 4);
		}


		auto* e = Entity::New();
		auto* gp = e->AddComponent<GregoryPatch>();
		for (int j = 0; j < 3; j++)
		{
			gp->neighbors[j] = &ordered[j]->owner;
			gp->neighborSide[j] = sides[j];
			ordered[j]->neighbors[sides[j]] = e;

			e->Register(ordered[j]->onModified, gp->NeighborModified);
		}

		static unsigned patchI = 1;
		std::string name = "Gregory Patch " + std::to_string(patchI++);
		e->SetName(name.c_str());
		outSurfs.push_back(e);
	}
	


	//e->Select(true);

	return;
}
