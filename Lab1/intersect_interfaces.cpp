#include "intersect_interfaces.h"
#include "intersect_math.h"
#include "bicubicSegment.h"
using namespace DirectX;

inline IntersectData<BicubicSegment>::IntersectData(BicubicSegment& object)
{
	for (int i = 0; i < 16; i++)
	{
		cpoints[i] = object.GetSource()->GetPoint(object.indices[i]);
	}
}

void IntersectData<BicubicSegment>::point(DirectX::XMFLOAT2 uv,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR& du,
	DirectX::XMVECTOR& dv)
{
	float bu[4], bv[4], dbu[4], dbv[4];

	bezierBasis(bu, dbu, uv.x);
	bezierBasis(bv, dbv, uv.y);

	position = du = dv = {};
	for (int v = 0; v < 4; v++)
		for (int u = 0; u < 4; u++)
		{
			position = position + bu[u] *  bv[v] * XMLoadFloat3(&cpoints[u + v * 4]);
			du = du +  dbu[u] *  bv[v] * XMLoadFloat3(&cpoints[u + v * 4]);
			dv = dv +   bu[u] * dbv[v] * XMLoadFloat3(&cpoints[u + v * 4]);
		}
}
