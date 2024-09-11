#include "entityPresets.h"
#include"all_components.h"
#include<string>
#include"vecmath.h"
using namespace DirectX;

Entity* EntityPresets::Torus(DirectX::XMFLOAT3 position)
{
	auto* e = Entity::New();
	auto& m = *e->AddComponent<TorusGenerator>();

	auto& t = *e->AddComponent<ModelTransform>();

	//e->AddComponent<MeshRenderer>()->mesh = Mesh::Torus(samples.x, samples.y, radii.x, radii.y).get();

	static unsigned modelI = 1;
	std::string name = "Torus " + std::to_string(modelI++);
	e->SetName(name.c_str());
	//e->Select(true);
	return e;
}

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

Entity* EntityPresets::IntersCurve(Entity* a, Entity* b, DxDevice& device,
	std::vector<DirectX::XMFLOAT2>& pointsA,
	std::vector<DirectX::XMFLOAT2>& pointsB)
{
	auto* e = Entity::New();
	auto* c = e->AddComponent<IntersectionCurve>();
	c->a = a; c->b = b;
	c->pointsA = pointsA; c->pointsB = pointsB;

	c->InitTextures(device, 128);

	static unsigned curveI = 1;
	std::string name = "Intersection Curve " + std::to_string(curveI++);
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
		assert(!(params.wrapMode.x) || params.division.x >= 3
			&& "Invalid segment count for cylinder");
		w = params.division.x + (params.wrapMode.x ? 0 : 3);
		h = params.division.y + 3;
	}
	else
	{
		w = params.division.x * 3 + (params.wrapMode.x ? 0 : 1);
		h = params.division.y * 3 + 1;
	}

	static unsigned surfI = 1;

	auto* surf = Entity::New();
	auto* pc = surf->AddComponent<PointCollection>();
	auto* collective = surf->AddComponent<BicubicSurface>();
	collective->division = params.division;
	collective->wrapMode = params.wrapMode;
	pc->isMutable = false;

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			DirectX::XMFLOAT3 offset;
			if (params.wrapMode == XMINT2{0,0})
				offset =
			{
				(x / (w - 1.0f) - 0.5f) * params.dimensions.x,
				(y / (h - 1.0f) - 0.5f) * params.dimensions.y,
				0
			};
			else
			{
				float skew = 0.9;
				int xnode = ((x+1) / 3) * 3;
				float sm1 = sin((xnode - skew) * DirectX::XM_2PI / w), cm1 = cos((xnode - skew) * DirectX::XM_2PI / w);
				float sp1 = sin((xnode + skew) * DirectX::XM_2PI / w), cp1 = cos((xnode + skew) * DirectX::XM_2PI / w);
				offset = { 0, (y / (h - 1.0f) - 0.5f) * params.dimensions.y , 0 };
				switch (params.deBoor?-1:(x % 3))
				{
				case -1:
					offset.x = sin((x)*DirectX::XM_2PI / w);
					offset.z = cos((x)*DirectX::XM_2PI / w);
					break;
				case 0:
					offset.x = (sm1 + sp1) / 2;
					offset.z = (cm1 + cp1) / 2;
					break;
				case 1:
					offset.x = sp1;
					offset.z = cp1;
					break;
				case 2:
					offset.x = sm1;
					offset.z = cm1;
					break;
				}
				offset.x *= params.dimensions.x;
				offset.z *= params.dimensions.x;
			}
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
	if (params.wrapMode.x)
	{
		for (int y = 0; y < params.division.y; y++)
		{
			auto* left = outSegments[params.division.x - 1 + y * params.division.x];
			auto* right = outSegments[0 + y * params.division.x];
			left->GetComponent<BicubicSegment>()->neighbors[1] = right;
			right->GetComponent<BicubicSegment>()->neighbors[3] = left;
		}
	}

	collective->SetSegments(outSegments);
	std::string name = "Bicubic Surface " + std::to_string(surfI);
	surf->SetName(name.c_str());
	surfI++;
	return surf;
}

void EntityPresets::FillInSurface(Entity::Selection s, std::vector<Entity*>& outSurfs)
{

	std::vector<BicubicSegment*> bss{};
	for (int i = 0; i < s.size(); i++)
	{
		auto* bs = s[i]->GetComponent<BicubicSegment>();
		if (bs && (PointCollection*)bs->GetSource())
		{
			bss.push_back(bs);
			if (bss.size() > 3)
				return;
		}
	}
	if (bss.size() < 3)
		return;


	std::array<std::array<Entity*, 4>, 3> corners;
	for (int j = 0; j < 3; j++)
		for (int i = 0; i < 4; i++)
			corners[j][i] = ((PointCollection*)bss[j]->GetSource())->entities[bss[j]->indices[BicubicSegment::corners[i]]];



	for (int startSide = 0; startSide < 4; startSide++)
	{
		if (bss[0]->neighbors[startSide] != nullptr)
			continue;

		std::vector<int> idx = { 0 }, side = { startSide };
		std::vector<bool> reverse = { false };

		auto boundary = bss[0]->GetBoundary(startSide);
		Entity* start = ((PointCollection*)bss[0]->GetSource())->entities[boundary[0]];
		Entity* end = ((PointCollection*)bss[0]->GetSource())->entities[boundary[3]];

		int pred = -1;
		{
			int corner = -1;
			for (int i = 0; i < 4; i++)
				if (corners[1][i] == end)
				{
					corner = i;
					pred = 1;
				}
			if (corner == -1)
				for (int i = 0; i < 4; i++)
					if (corners[2][i] == end)
					{
						corner = i;
						pred = 2;
					}
			if (corner == -1)
				continue;

			int succ = 3 - pred;
			auto makePatch = [&]()
				{
					auto* e = Entity::New();
					auto* gp = e->AddComponent<GregoryPatch>();
					for (int j = 0; j < 3; j++)
					{
						gp->neighbors[j] = &bss[idx[2-j]]->owner;
						gp->neighborSide[j] = side[2-j];
						gp->neighborReverse[j] = reverse[2-j];

						bss[idx[j]]->neighbors[side[2-j]] = e;

						e->Register(bss[idx[j]]->onModified, gp->NeighborModified);
					}

					static unsigned patchI = 1;
					std::string name = "Gregory Patch " + std::to_string(patchI++);
					e->SetName(name.c_str());
					outSurfs.push_back(e);
				};

			auto endPatch = [&]()
				{
					int cend = -99999;
					for (int i = 0; i < 4; i++)
						if (corners[succ][i] == start)
							cend = i;
					int cstart = -99999;
					for (int i = 0; i < 4; i++)
						if (corners[succ][i] == end)
							cstart = i;
					if (cend - cstart == 1 || cend - cstart == -3)
					{
						side.push_back(cstart);
						reverse.push_back(false);
						idx.push_back(succ);
						makePatch();
						side.pop_back();
						reverse.pop_back();
						idx.pop_back();
					}
					if (cend - cstart == -1 || cend - cstart == 3)
					{
						side.push_back(cend);
						reverse.push_back(true);
						idx.push_back(succ);
						makePatch();
						side.pop_back();
						reverse.pop_back();
						idx.pop_back();
					}
				};

			int edge = corner;
			if (bss[pred]->neighbors[edge] == nullptr)
			{
				side.push_back(edge);
				reverse.push_back(false);
				idx.push_back(pred);
				boundary = bss[pred]->GetBoundary(edge);
				end = ((PointCollection*)bss[pred]->GetSource())->entities[boundary[3]];
				endPatch();
				side.pop_back();
				reverse.pop_back();
				idx.pop_back();
			}
			edge = (corner + 3) % 4;
			if (bss[pred]->neighbors[edge] == nullptr)
			{
				side.push_back(edge);
				reverse.push_back(true);
				idx.push_back(pred);
				boundary = bss[pred]->GetBoundary(edge);
				end = ((PointCollection*)bss[pred]->GetSource())->entities[boundary[0]];
				endPatch();
				side.pop_back();
				reverse.pop_back();
				idx.pop_back();
			}
		}
	}

	return;
}
