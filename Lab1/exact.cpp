#include "exact.h"

#include "intersect_interfaces.h"
#include"intersect.h"
#include"bicubicSurface.h"
#include"torusGenerator.h"

static const int headDiameter = 8;
static const int baseWidth = 150;
static const int baseHeight = 50-16;
static const float r = headDiameter / 2.0f / baseWidth,
r2 = r * r;
static const float heightMultiplier = (float)baseHeight / baseWidth;

using namespace DirectX;
void SingleExact(Entity* object, float precision, std::vector<DirectX::XMFLOAT3>& path)
{
	auto* surf = object->GetComponent<BicubicSurface>();
	if (!surf)
		return;
	auto d1 = Inflation<BicubicSurface>(surf, r);
	//auto d2 = IntersectData<Inflation<BicubicSurface>>(&d1);
}

XMFLOAT3 hardCursors[] = {
	{0.150f, -2.722f, 0.0f},
};
void Paths::Exact(Entity::Selection& set, float precision, std::vector<DirectX::XMFLOAT3>& path)
{
	std::vector<Inflation<BicubicSurface>> surf;
	std::vector<Inflation<TorusGenerator>> tors;

	for(int i = 0; i<set.size(); i++)
	{
		auto* s = set[i]->GetComponent<BicubicSurface>();
		auto* t = set[i]->GetComponent<TorusGenerator>();
		if (s)
			surf.push_back({ s,r });
		if (t)
			tors.push_back({ t,r });
	}
	std::vector<XMFLOAT3> pts;
	std::vector<XMFLOAT2> uva, uvb;
	std::vector < std::vector<XMFLOAT2>> shapes;
	for (int i = 0; i < surf.size(); i++)
	{
		bool loop;
		if(intersect(&surf[0], &surf[i], SceneCursor::instance.GetWorld(),
			precision, pts, uva, uvb, loop))
		{
			shapes.push_back(uvb);
			path.insert(path.end(), pts.begin(), pts.end());
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
}
