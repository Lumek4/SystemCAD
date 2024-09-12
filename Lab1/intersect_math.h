#pragma once
#include"intersect_interfaces.h"
#include"vecmath.h"
#include<DirectXMath.h>

inline bool AABB(const BoundingBox& o1, const BoundingBox& o2)
{
	using namespace DirectX;
	XMVECTOR v1 = XMVectorLessOrEqual(o1.hi, o2.lo);
	XMVECTOR v2 = XMVectorLessOrEqual(o2.hi, o1.lo);
	XMVECTOR v3 = XMVector3Dot(XMConvertVectorIntToFloat(
		XMVectorOrInt(v1, v2),0), { 1.0f,1.0f,1.0f });
	return XMVectorGetX(v3) == 0;
}

inline void bezierBasis(float b3[4], float db3[4], float t)
{
	float b2[3], b1[2];

	float mt = 1 - t;
	b1[0] = mt;
	b1[1] = t;

	b2[0] = b1[0] * mt;
	b2[1] = b1[0] * t + b1[1] * mt;
	b2[2] = b1[1] * t;

	b3[0] = b2[0] * mt;
	b3[1] = b2[0] * t + b2[1] * mt;
	b3[2] = b2[1] * t + b2[2] * mt;
	b3[3] = b2[2] * t;

	db3[0] = -b2[0] * 3;
	db3[1] = (b2[0] - b2[1]) * 3;
	db3[2] = (b2[1] - b2[2]) * 3;
	db3[3] = b2[2] * 3;
}