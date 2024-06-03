#include "bezierCurve.h"
#include"catalogue.hpp"
#include"entity.hpp"
#include"vPointCollection.h"
#include"vecmath.h"

void BezierCurve::NeedsRedraw_PointCollectionFunction(void* arg, NeedRedrawEventData)
{
	((BezierCurve*)arg)->modified = true;
}
BezierCurve::BezierCurve(Entity& owner)
	:Component(ComponentConstructorArgs(BezierCurve)),
	NeedsRedraw_PointCollection(this, NeedsRedraw_PointCollectionFunction)
{
	pointSource = owner.GetComponent<VPointCollection>();
	if(!pointSource)
		pointSource = &RequireComponent(PointSource);

	owner.Register(pointSource->NeedRedrawEvent(), NeedsRedraw_PointCollection);
}

Mesh* BezierCurve::GetMesh()
{
	if (!modified)
		return m.get();
	int count = pointSource->GetCount();
	int virtualCount = (3 - ((count - 1) % 3)) % 3;
	modified = false;

	std::vector<VertexPosition> vv; 
	std::vector<unsigned> ii;
	for (int i = 0; i < count + virtualCount; i++)
	{
		ii.emplace_back(vv.size());
		if (i % 3 == 0 && i != 0 && i != count + virtualCount - 1)
			ii.emplace_back(vv.size());
		vv.emplace_back(pointSource->GetPoint(i));
	}
	swap(vv.back(), vv[count - 1]);

	m = std::make_unique<Mesh>(vv, ii, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	return m.get();

}