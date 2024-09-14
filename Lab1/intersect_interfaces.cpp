#include "intersect_interfaces.h"
#include "intersect_math.h"
#include "bicubicSegment.h"
#include "bicubicSurface.h"
#include "torusGenerator.h"
#include "transform.h"
using namespace DirectX;

IntersectData<BicubicSegment>::IntersectData(BicubicSegment* object)
{
	XMVECTOR p[16];
	for (int i = 0; i < 16; i++)
	{
		XMFLOAT3 tmp = object->GetSource()->GetPoint(object->indices[i]);
		p[i] = XMLoadFloat3(&tmp);
	}

	if (object->deBoorMode)
	{
		XMVECTOR extBezier[16];
		for (int y = 0; y < 4; y++)
		{
			extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 2.0 / 3);
			extBezier[1 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 1.0 / 3);
			extBezier[2 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 2.0 / 3);
			extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 1.0 / 3);
		}
		for (int x = 0; x < 4; x++)
		{
			p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 2.0 / 3);
			p[x + 1 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 1.0 / 3);
			p[x + 2 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 2.0 / 3);
			p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 1.0 / 3);
		}

		for (int y = 0; y < 4; y++)
		{
			extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 0.5);
			extBezier[1 + y * 4] = p[1 + y * 4];
			extBezier[2 + y * 4] = p[2 + y * 4];
			extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 0.5);
		}
		for (int x = 0; x < 4; x++)
		{
			p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 0.5);
			p[x + 1 * 4] = extBezier[x + 1 * 4];
			p[x + 2 * 4] = extBezier[x + 2 * 4];
			p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 0.5);
		}
	}
	bb.hi = -XMVectorSplatInfinity();
	bb.lo = XMVectorSplatInfinity();
	for (int i = 0; i < 16; i++)
	{
		XMStoreFloat3(&cpoints[i], p[i]);
		bb.hi = XMVectorMax(bb.hi, p[i]);
		bb.lo = XMVectorMin(bb.lo, p[i]);
	}
}

void IntersectData<BicubicSegment>::Point(DirectX::XMFLOAT2 uv,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR& du,
	DirectX::XMVECTOR& dv) const
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

IntersectData<BicubicSurface>::IntersectData(BicubicSurface* object)
{
	division = object->division;
	wrapMode = object->wrapMode;
	bb.hi = -XMVectorSplatInfinity();
	bb.lo = XMVectorSplatInfinity();
	auto& segs = object->GetSegments();
	for (int j = 0; j < segs.size(); j++)
	{
		cpoints.push_back({});
		XMVECTOR p[16];
		for (int i = 0; i < 16; i++)
		{
			XMFLOAT3 tmp = segs[j]->GetSource()->GetPoint(segs[j]->indices[i]);
			p[i] = XMLoadFloat3(&tmp);
		}

		if (segs[j]->deBoorMode)
		{
			XMVECTOR extBezier[16];
			for (int y = 0; y < 4; y++)
			{
				extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 2.0 / 3);
				extBezier[1 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 1.0 / 3);
				extBezier[2 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 2.0 / 3);
				extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 1.0 / 3);
			}
			for (int x = 0; x < 4; x++)
			{
				p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 2.0 / 3);
				p[x + 1 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 1.0 / 3);
				p[x + 2 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 2.0 / 3);
				p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 1.0 / 3);
			}

			for (int y = 0; y < 4; y++)
			{
				extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 0.5);
				extBezier[1 + y * 4] = p[1 + y * 4];
				extBezier[2 + y * 4] = p[2 + y * 4];
				extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 0.5);
			}
			for (int x = 0; x < 4; x++)
			{
				p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 0.5);
				p[x + 1 * 4] = extBezier[x + 1 * 4];
				p[x + 2 * 4] = extBezier[x + 2 * 4];
				p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 0.5);
			}
		}
		for (int i = 0; i < 16; i++)
		{
			XMStoreFloat3(&cpoints[j][i], p[i]);
			bb.hi = XMVectorMax(bb.hi, p[i]);
			bb.lo = XMVectorMin(bb.lo, p[i]);
		}
	}
}
void IntersectData<BicubicSurface>::Point(DirectX::XMFLOAT2 uv,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR& du,
	DirectX::XMVECTOR& dv) const
{
	XMINT2 suv
	{
		(int32_t)(uv.x * division.x),
		(int32_t)(uv.y * division.y)
	};
	uv.x = fmodf(uv.x*division.x, 1.0f);
	uv.y = fmodf(uv.y*division.y, 1.0f);
	if (suv.x == division.x && uv.x == 0)
	{
		suv.x -= 1;
		uv.x += 1.0f;
	}
	if (suv.y == division.y && uv.y == 0)
	{
		suv.y -= 1;
		uv.y += 1.0f;
	}

	auto& scpoints = cpoints[suv.x + suv.y * division.x];

	float bu[4], bv[4], dbu[4], dbv[4];

	bezierBasis(bu, dbu, uv.x);
	bezierBasis(bv, dbv, uv.y);

	position = du = dv = {};
	for (int v = 0; v < 4; v++)
		for (int u = 0; u < 4; u++)
		{
			position = position + bu[u] * bv[v] * XMLoadFloat3(&scpoints[u + v * 4]);
			du = du + dbu[u] * bv[v] * XMLoadFloat3(&scpoints[u + v * 4]);
			dv = dv + bu[u] * dbv[v] * XMLoadFloat3(&scpoints[u + v * 4]);
		}
	du *= division.x;
	dv *= division.y;
}

IntersectData<TorusGenerator>::IntersectData(TorusGenerator* object)
{
	auto t = object->owner.GetComponent<Transform>();
	radii = object->m_meshData.radii;
	auto lv = t->Position();
	auto v = XMLoadFloat3(&lv);
	auto extent = XMVECTOR{ 1, 1, 1, 0 } * (radii.x + radii.y) * t->scale.x;
	bb.hi = v + extent;
	bb.lo = v - extent;
	transform = t->Get();
}

void IntersectData<TorusGenerator>::Point(DirectX::XMFLOAT2 uv,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR& du,
	DirectX::XMVECTOR& dv) const
{
	uv.x = uv.x * XM_2PI - XM_PI;
	uv.y = uv.y * XM_2PI - XM_PI;
	position = {
		(radii.x + radii.y * cosf(uv.y)) * cosf(uv.x),
		(radii.x + radii.y * cosf(uv.y)) * sinf(uv.x),
		radii.y * sinf(uv.y),
		1
	};
	du = {
		(radii.x + radii.y * cosf(uv.y)) *-sinf(uv.x) * XM_2PI,
		(radii.x + radii.y * cosf(uv.y)) * cosf(uv.x) * XM_2PI,
		0,
		0
	};
	dv = {
		radii.y * -sinf(uv.y) * cosf(uv.x) * XM_2PI,
		radii.y * -sinf(uv.y) * sinf(uv.x) * XM_2PI,
		radii.y * cosf(uv.y) * XM_2PI,
		0
	};
	position = XMVector4Transform(position, transform);
	du = XMVector4Transform(du, transform);
	dv = XMVector4Transform(dv, transform);
}