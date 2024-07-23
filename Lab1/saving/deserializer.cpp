#include "deserializer.h"
#include"../all_components.h"

using namespace DirectX;

Entity* Deserializer::Point(const std::string& name, DirectX::XMFLOAT3 position)
{
	auto* e = Entity::New();
	auto* t = e->AddComponent<PointTransform>();
	t->Translate(position);
	e->AddComponent<PointRenderer>();
	e->SetName(name.c_str());
	return e;
}

Entity* Deserializer::Torus(const std::string& name, DirectX::XMFLOAT2 radii, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMINT2 samples)
{
	auto* e = Entity::New();
	auto& m = *e->AddComponent<TorusGenerator>();
	m.SetData({samples, radii});

	auto& t = *e->AddComponent<ModelTransform>();
	t.Translate(position);
	t.scale = scale;
	t.Rotate(position, { 1,0,0 }, rotation.x);
	t.Rotate(position, { 0,0,1 }, rotation.y);
	t.Rotate(position, { 0,1,0 }, rotation.z);

	//e->AddComponent<MeshRenderer>()->mesh = Mesh::Torus(samples.x, samples.y, radii.x, radii.y).get();

	e->SetName(name.c_str());
	//e->Select(true);
	return e;
}

Entity* Deserializer::BezierC0(const std::string& name, const Entity::Selection& controlPoints)
{
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(controlPoints);
	e->AddComponent<BezierCurve>();

	e->SetName(name.c_str());

	return e;
}

Entity* Deserializer::BezierC2(const std::string& name, const Entity::Selection& deBoorPoints)
{
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(deBoorPoints);
	e->AddComponent<VPointCollection>();
	e->AddComponent<SplineGenerator>();
	e->AddComponent<BezierCurve>();

	e->SetName(name.c_str());

	return e;
}

Entity* Deserializer::InterpolatedC2(const std::string& name, const Entity::Selection& controlPoints)
{
	auto* e = Entity::New();
	e->AddComponent<PointCollection>()->AddSelection(controlPoints);
	e->AddComponent<VPointCollection>()->ShowVirtualPoints(false);
	e->AddComponent<BezierInterpolator>();
	e->AddComponent<BezierCurve>();

	e->SetName(name.c_str());

	return e;
}

Entity* Deserializer::BezierSurfaceC0(
	Entity::Selection& outSegments,
	const std::string& name,
	const std::vector<std::string*>& segmentNames,
	const Entity::Selection& controlPoints,
	DirectX::XMINT2 parameterWrapped,
	DirectX::XMINT2 size)
{
	auto* e = Entity::New();
	auto* pc = e->AddComponent<PointCollection>();
	auto* bs = e->AddComponent<BicubicSurface>();

	pc->AddSelection(controlPoints);
	pc->isMutable = false;
	bs->division = size;
	bs->wrapMode = parameterWrapped;

	const auto& collectionPoints = pc->Get();
	auto it_b = collectionPoints.begin(), it_e = collectionPoints.end();

	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
		{
			auto* e = Entity::New();
			auto* bcs = e->AddComponent<BicubicSegment>();
			bcs->SetSource(pc);
			bcs->deBoorMode = false;
			for (int yy = 0; yy < 4; yy++)
			for (int xx = 0; xx < 4; xx++)
				{
					auto it = std::find(it_b, it_e,
						controlPoints[xx+yy*4 + (y+x*size.y)*16]);
					bcs->indices[xx+yy*4] = it - it_b;
				}
			e->SetName(*segmentNames[y + size.y * x]);
			outSegments.push_back(e);
		}

	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
		{
			if (y != 0)
			{
				auto* top = outSegments[(y - 1) + x * size.y];
				auto* bot = outSegments[y + x * size.y];
				top->GetComponent<BicubicSegment>()->neighbors[2] = bot;
				bot->GetComponent<BicubicSegment>()->neighbors[0] = top;
			}
			if (x != 0)
			{
				auto* left = outSegments[y + (x - 1) * size.y]; 
				auto* right = outSegments[y + x * size.y];      
				left->GetComponent<BicubicSegment>()->neighbors[1] = right;
				right->GetComponent<BicubicSegment>()->neighbors[3] = left;
			}
		}
	if (parameterWrapped.y)
	{
		for (int y = 0; y < size.y; y++)
		{
			auto* left = outSegments[y + (size.x - 1) * size.y];
			auto* right = outSegments[y + 0 * size.y];
			left->GetComponent<BicubicSegment>()->neighbors[1] = right;
			right->GetComponent<BicubicSegment>()->neighbors[3] = left;
		}
	}
	if (parameterWrapped.x)
	{
		for (int x = 0; x < size.x; x++)
		{
			auto* top = outSegments[size.y - 1 + x * size.y]; 
			auto* bot = outSegments[0 + x * size.y];          
			top->GetComponent<BicubicSegment>()->neighbors[2] = bot;
			bot->GetComponent<BicubicSegment>()->neighbors[0] = top;
		}
	}
	bs->SetSegments(outSegments);
	e->SetName(name.c_str());
	return e;
}

Entity* Deserializer::BezierSurfaceC2(Entity::Selection& outSegments, const std::string& name, const std::vector<std::string*>& segmentNames, const Entity::Selection& controlPoints, DirectX::XMINT2 parameterWrapped, DirectX::XMINT2 size)
{
	auto* e = Entity::New();
	auto* pc = e->AddComponent<PointCollection>();
	auto* bs = e->AddComponent<BicubicSurface>();

	pc->AddSelection(controlPoints);
	pc->isMutable = false;

	bs->division = size;
	bs->wrapMode = parameterWrapped;

	const auto& collectionPoints = pc->Get();
	auto it_b = collectionPoints.begin(), it_e = collectionPoints.end();

	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
		{
			auto* e = Entity::New();
			auto* bcs = e->AddComponent<BicubicSegment>();
			bcs->SetSource(pc);
			bcs->deBoorMode = true;
			for (int yy = 0; yy < 4; yy++)
				for (int xx = 0; xx < 4; xx++)
				{
					auto it = std::find(it_b, it_e,
						controlPoints[xx + yy * 4 + (y + x * size.y) * 16]);
					bcs->indices[xx + yy * 4] = it - it_b;
				}
			e->SetName(*segmentNames[y + size.y * x]);
			outSegments.push_back(e);
		}

	bs->SetSegments(outSegments);
	e->SetName(name.c_str());
	return e;
}
