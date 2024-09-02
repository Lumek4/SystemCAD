#include "intersect.h"
#include"vecmath.h"
#include"bicubicSegment.h"
using namespace DirectX;


bool overlaps(float a1, float a2, float b1, float b2)
{
	return !(a2 - b1 <= EPS || b2 - a1 <= EPS);
}
bool bboxcheck(std::vector<XMFLOAT3> pa, std::vector<XMFLOAT3> pb)
{
	XMFLOAT3 amin, amax, bmin, bmax;
	assert(!pa.empty());
	assert(!pb.empty());
	amin = amax = pa[0];
	bmin = bmax = pb[0];
	assert(pa.size() == pb.size());
	for (int i = 1; i < pa.size(); i++)
	{
		amin.x = fminf(pa[i].x, amin.x);
		amax.x = fmaxf(pa[i].x, amax.x);
		amin.y = fminf(pa[i].y, amin.y);
		amax.y = fmaxf(pa[i].y, amax.y);
		amin.z = fminf(pa[i].z, amin.z);
		amax.z = fmaxf(pa[i].z, amax.z);

		bmin.x = fminf(pb[i].x, bmin.x);
		bmax.x = fmaxf(pb[i].x, bmax.x);
		bmin.y = fminf(pb[i].y, bmin.y);
		bmax.y = fmaxf(pb[i].y, bmax.y);
		bmin.z = fminf(pb[i].z, bmin.z);
		bmax.z = fmaxf(pb[i].z, bmax.z);
	}
	bool ox = overlaps(amin.x, amax.x, bmin.x, bmax.x);
	bool oy = overlaps(amin.y, amax.y, bmin.y, bmax.y);
	bool oz = overlaps(amin.z, amax.z, bmin.z, bmax.z);
	return ox && oy && oz;
}
void bezierBasis(float b3[4], float db3[4], float t)
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
void bezierPatchPoint(std::vector<XMFLOAT3> pts, float u, float v,
	XMVECTOR& pos, XMVECTOR& du, XMVECTOR& dv)
{
	float bu[4], bv[4], dbu[4], dbv[4];

	bezierBasis(bu, dbu, u);
	bezierBasis(bv, dbv, v);

	pos = du = dv = {};
	for (int v = 0; v < 4; v++)
		for (int u = 0; u < 4; u++)
		{
			pos = pos + bu[u] * bv[v] * XMLoadFloat3(&pts[u + v * 4]);
			du  = du + dbu[u] * bv[v] * XMLoadFloat3(&pts[u + v * 4]);
			dv  = dv + bu[u] * dbv[v] * XMLoadFloat3(&pts[u + v * 4]);
		}
}
XMFLOAT3 bezierPatchPointGrid(std::vector<XMFLOAT3> pts, int u, int v, int max)
{
	static bool precalc[100] = { false };
	static float bases[100][4];
	//static float dbases[100][4];
	float bu[4], bv[4], db[4];

	if (!precalc[u])
	{
		bezierBasis(bases[u], db, u*1.0f/max);
		precalc[u] = true;
	}
	memcpy(bu, bases[u], sizeof(bu));
	//memcpy(dbu, dbases[u], sizeof(dbu));
	if (!precalc[v])
	{
		bezierBasis(bases[v], db, v*1.0f/max);
		precalc[v] = true;
	}
	memcpy(bv, bases[v], sizeof(bv));
	//memcpy(dbv, dbases[v], sizeof(dbv));

	//XMFLOAT3 du, dv;
	XMFLOAT3 pos = XMFLOAT3();
	for (int v = 0; v < 4; v++)
		for (int u = 0; u < 4; u++)
		{
			pos = pos + bu[u] * bv[v] * pts[u + v * 4];
			//du = du + dbu[u] * bv[v] * pts[u + v * 4];
			//dv = dv + bu[u] * dbv[v] * pts[u + v * 4];
		}
	//normal = XMFLOAT3{
	//	du.y * dv.z - du.z * dv.y,
	//	du.z * dv.x - du.x * dv.z,
	//	du.x * dv.y - du.y * dv.x };
	//normal = normal * (1 / sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z));
	return pos;
}

XMVECTOR nearCursorPoints(
	const std::vector<XMFLOAT3>& pa,
	const std::vector<XMFLOAT3>& pb,
	const XMFLOAT3& cursor)
{
	constexpr int maxres = 10;
	float mindista = INFINITY;
	XMINT2 mincoorda;
	for (int ua = 0; ua < maxres; ua++)
		for (int va = 0; va < maxres; va++)
		{
			XMFLOAT3 ppa = bezierPatchPointGrid(pa, ua, va, maxres);
			float len = vecmath::lengthSq(ppa - cursor);
			if (len < mindista)
			{
				mincoorda = { ua, va };
				mindista = len;
			}
		}
	float mindistb = INFINITY;
	XMINT2 mincoordb;
	for (int ub = 0; ub < maxres; ub++)
		for (int vb = 0; vb < maxres; vb++)
		{
			XMFLOAT3 ppb = bezierPatchPointGrid(pb, ub, vb, maxres);
			float len = vecmath::lengthSq(ppb - cursor);
			if (len < mindistb)
			{
				mincoordb = { ub, vb };
				mindistb = len;
			}
		}

	return {
		mincoorda.x * 1.0f / maxres,
		mincoorda.y * 1.0f / maxres,
		mincoordb.x * 1.0f / maxres,
		mincoordb.y * 1.0f / maxres
	};
}
XMVECTOR pullBack(XMVECTOR vector, XMMATRIX j)
{
	// 	        
	//               [aux  avx -bux -bvx]     [au]	                 [bx-ax]
	// (AtA)^-1*At * [auy  avy -buy -bvy]  *  [av]  =  (AtA)^-1*At * [by-ay]
	//               [auz  avz -buz -bvz]     [bu]	                 [bz-az]
	//         ^Identity^                     [bv]
	auto jp = XMMatrixTranspose(j) * j;
	jp.r[3].m128_f32[3] = 1;

	auto invA = j * XMMatrixInverse(nullptr, jp);
	auto identity = j * XMMatrixTranspose(invA);
	return XMVector4Transform(vector, XMMatrixTranspose(invA));
}
bool intersect(BicubicSegment* a, BicubicSegment* b,
	std::vector<DirectX::XMFLOAT3>& output, XMFLOAT3 cursor)
{
	auto pa = std::vector<XMFLOAT3>();
	auto pb = std::vector<XMFLOAT3>();
	for (int i = 0; i < 16; i++)
	{
		pa.push_back(a->GetSource()->GetPoint(a->indices[i]));
		pb.push_back(b->GetSource()->GetPoint(b->indices[i]));
	}
	if (!bboxcheck(pa, pb))
		return false;

	auto coord = nearCursorPoints(pa, pb, cursor);
	float dist = INFINITY;
	XMMATRIX mmb, mma;
	XMVECTOR apos, adu, adv;
	XMVECTOR bpos, bdu, bdv;
	int it;
	for (int it = 1; ; it++, coord = XMVectorClamp(coord +
		pullBack(bpos - apos, { adu, adv, -bdu, -bdv }),
			{ 0,0,0,0 }, { 1,1,1,1 })
		)
	{
		bezierPatchPoint(pa, XMVectorGetX(coord), XMVectorGetY(coord),
			apos, adu, adv);
		bezierPatchPoint(pb, XMVectorGetZ(coord), XMVectorGetW(coord),
			bpos, bdu, bdv);


		dist = XMVectorGetX(XMVector3Length(bpos - apos));
		if (dist < 1e-3)
			break;
		else if (it >= 150)
			return false;
	}

	output.push_back({});
	XMStoreFloat3(&output.back(), apos);
	std::vector <XMFLOAT4> cds;
	auto initialCoord = coord;
	for (int sgn = -1; sgn <= 1; sgn += 2)
	{
		coord = initialCoord;
		bezierPatchPoint(pa, XMVectorGetX(coord), XMVectorGetY(coord),
			apos, adu, adv);
		bezierPatchPoint(pb, XMVectorGetZ(coord), XMVectorGetW(coord),
			bpos, bdu, bdv);
		for (int it = 1; it < 1000; it++)
		{
			auto tangent = sgn * XMVector3Normalize(XMVector3Cross(
				XMVector3Cross(adu, adv),
				XMVector3Cross(bdu, bdv)
			)) / 15;
			coord = coord + pullBack(tangent, { adu, adv, -bdu, -bdv });

			if (XMVectorGetX(
				XMVectorSum(
					XMVectorAdd(
						XMVectorGreater(coord, XMVectorSplatOne()),
						XMVectorGreater(XMVectorZero(), coord)
					)
				)
			))
				break;

			bezierPatchPoint(pa, XMVectorGetX(coord), XMVectorGetY(coord),
				apos, adu, adv);
			bezierPatchPoint(pb, XMVectorGetZ(coord), XMVectorGetW(coord),
				bpos, bdu, bdv);
			output.push_back({});
			XMStoreFloat3(&output.back(), apos);
			cds.push_back({});
			XMStoreFloat4(&cds.back(), coord);
		}
	}

	return true;
}
