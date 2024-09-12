#pragma once
#include"bicubicSegment.h"
#include"intersect_interfaces.h"
#include"intersect_math.h"

template<typename T, int maxres>
DirectX::XMFLOAT2 nearCursorPoint(
	const IntersectData<T>& data,
	const DirectX::XMFLOAT3& cursor)
{
	using namespace DirectX;
	float mindist = INFINITY;
	XMINT2 mincoord;
	XMVECTOR cur = XMLoadFloat3(&cursor);
	for (int u = 0; u < maxres; u++)
		for (int v = 0; v < maxres; v++)
		{
			XMVECTOR pos, du, dv;
			data.Point({ u * 1.0f / maxres, v * 1.0f / maxres }, pos, du, dv);
			float len = XMVectorGetX(XMVector3LengthSq(pos - cur));
			if (len < mindist)
			{
				mincoord = { u, v };
				mindist = len;
			}
		}

	return {
		mincoord.x * 1.0f / maxres,
		mincoord.y * 1.0f / maxres
	};
}
DirectX::XMVECTOR pullBack(
	DirectX::XMVECTOR vector,
	DirectX::XMMATRIX j,
	bool& error)
{
	using namespace DirectX;
	// 	        
	//               [aux  avx -bux -bvx]     [au]	                 [bx-ax]
	// (AtA)^-1*At * [auy  avy -buy -bvy]  *  [av]  =  (AtA)^-1*At * [by-ay]
	//               [auz  avz -buz -bvz]     [bu]	                 [bz-az]
	//         ^Identity^                     [bv]
	auto jp = XMMatrixTranspose(j) * j;
	jp.r[3].m128_f32[3] = 1;
	XMVECTOR det;
	auto invA = j * XMMatrixInverse(&det, jp);
	auto identity = j * XMMatrixTranspose(invA);
	error = XMVectorGetX(det) == 0 ? true : false;
	return XMVector4Transform(vector, XMMatrixTranspose(invA));
}

template<typename TA, typename TB>
bool converge(
	const IntersectData<TA>& da,
	const IntersectData<TB>& db,
	DirectX::XMVECTOR& point,
	bool& error,
	int iterations = 150
)
{
	using namespace DirectX;
	XMVECTOR apos, adu, adv;
	XMVECTOR bpos, bdu, bdv;
	XMVECTOR pt1{}, pt2{};
	XMVECTOR clampLo = {
		da.GetWrapMode().x ? -INFINITY : 0,
		da.GetWrapMode().y ? -INFINITY : 0,
		db.GetWrapMode().x ? -INFINITY : 0,
		db.GetWrapMode().y ? -INFINITY : 0,
	};
	XMVECTOR clampHi = {
		da.GetWrapMode().x ? INFINITY : 1,
		da.GetWrapMode().y ? INFINITY : 1,
		db.GetWrapMode().x ? INFINITY : 1,
		db.GetWrapMode().y ? INFINITY : 1,
	};
	XMVECTOR ones{ 1,1,1,1 };
	for (int it = 1; ; it++, point +=
		pullBack(bpos - apos, { adu, adv, -bdu, -bdv }, error)*0.5f
		)
	{
		if (error)
			return false;
		/*if (it >= 3)
			if (XMVectorGetX(XMVector3Length((point + pt2) / 2 - pt1)) >
				XMVectorGetX(XMVector3Length(point - pt2)) * 10)
				point = (point + pt1) / 2;*/


		point = XMVectorClamp(point,
			clampLo, clampHi);
		point = XMVectorSelect(point, point - ones, XMVectorGreater(point, ones));
		point = XMVectorSelect(point, point + ones, XMVectorLess(point, XMVectorZero()));
		da.Point({ XMVectorGetX(point), XMVectorGetY(point) },
			apos, adu, adv);
		db.Point({ XMVectorGetZ(point), XMVectorGetW(point) },
			bpos, bdu, bdv);


		float dist = XMVectorGetX(XMVector3Length(bpos - apos));
		if (dist < 1e-3)
			return true;
		else if (it >= iterations)
			return false;
		pt2 = pt1;
		pt1 = point;
	}
}
template<typename TA, typename TB>
bool intersect(TA* a, TB* b,
	DirectX::XMFLOAT3 cursor, float precision,
	std::vector<DirectX::XMFLOAT3>& output,
	std::vector<DirectX::XMFLOAT2>& uva,
	std::vector<DirectX::XMFLOAT2>& uvb,
	bool& loop)
{
	using namespace DirectX;
	auto da = IntersectData<TA>(a);
	auto db = IntersectData<TB>(b);
	if (!AABB(da.GetBB(), db.GetBB()))
		return false;

	XMVECTOR startCoord;
	{
		auto ca = nearCursorPoint<TA, 128>(da, cursor);
		auto cb = nearCursorPoint<TB, 128>(db, cursor);
		startCoord = { ca.x, ca.y, cb.x, cb.y };
	}
	float dist = INFINITY;
	int it = 0;
	bool error = false;
	while(!converge(da, db, startCoord, error))
	{
		it++;
		if (it > 1000)
			return false;
		startCoord = {
			rand()*1.0f/RAND_MAX,
			rand()*1.0f/RAND_MAX,
			rand()*1.0f/RAND_MAX,
			rand()*1.0f/RAND_MAX
		};
	}

	XMVECTOR apos, adu, adv;
	XMVECTOR bpos, bdu, bdv;
	da.Point({ XMVectorGetX(startCoord), XMVectorGetY(startCoord) },
		apos, adu, adv);
	db.Point({ XMVectorGetZ(startCoord), XMVectorGetW(startCoord) },
		bpos, bdu, bdv);
	XMVECTOR coord;
	XMVECTOR initialPos = apos;
	std::vector <XMFLOAT4> cds[2];
	cds[0].push_back({}); XMStoreFloat4(&cds[0].back(), startCoord);
	std::vector <XMFLOAT3> pts[2];
	pts[0].push_back({}); XMStoreFloat3(&pts[0].back(), initialPos);
	for (int sgn = -1; sgn <= 1; sgn += 2)
	{
		coord = startCoord;
		da.Point({ XMVectorGetX(coord), XMVectorGetY(coord) },
			apos, adu, adv);
		db.Point({ XMVectorGetZ(coord), XMVectorGetW(coord) },
			bpos, bdu, bdv);
		for (int it = 1; it < 1000; it++)
		{
			auto tangent = sgn * XMVector3Normalize(XMVector3Cross(
				XMVector3Cross(adu, adv),
				XMVector3Cross(bdu, bdv)
			)) * precision;
			bool error;
			coord = coord + pullBack(tangent, { adu, adv, bdu, bdv }, error);
			if(error)
				return false;
			converge(da, db, coord, error, 10);
			if (error)
				return false;

			da.Point({ XMVectorGetX(coord), XMVectorGetY(coord) },
				apos, adu, adv);
			db.Point({ XMVectorGetZ(coord), XMVectorGetW(coord) },
				bpos, bdu, bdv);

			pts[(sgn + 1) / 2].push_back({});
			XMStoreFloat3(&pts[(sgn + 1) / 2].back(), apos);
			cds[(sgn + 1) / 2].push_back({});
			XMStoreFloat4(&cds[(sgn + 1) / 2].back(), coord);

			if (XMVectorGetX(
				XMVector4Dot(XMConvertVectorIntToFloat(
					XMVectorAdd(
						XMVectorGreaterOrEqual(coord, XMVectorSplatOne()),
						XMVectorGreaterOrEqual(XMVectorZero(), coord)
					), 0),
					{
						da.GetWrapMode().x ? 0.0f : -1.0f, da.GetWrapMode().y ? 0.0f : -1.0f,
						db.GetWrapMode().x ? 0.0f : -1.0f, db.GetWrapMode().y ? 0.0f : -1.0f
					}
				)
			))
				break;

			if (it > 2 && sgn == -1
				&& XMVectorGetX(XMVector3LengthSq(
					apos - XMLoadFloat3(&pts[0][0])))
				+ XMVectorGetX(XMVector3LengthSq(
					apos - XMLoadFloat3(&pts[0][1]))) <
				XMVectorGetX(XMVector3LengthSq(
					XMLoadFloat3(&pts[0][0]) - XMLoadFloat3(&pts[0][1])))
				)
			{
				loop = true;
				sgn = 2;
				break;
			}
		}
	}
	for (int i = 0; i < pts[0].size(); i++)
		output.push_back(pts[0][pts[0].size() - 1 - i]);
	for (int i = 0; i < pts[1].size(); i++)
		output.push_back(pts[1][i]);

	//CHECK IF last x - current x ~ 1, then
	// last x - limit, limit - current x
	// TO DO, IMPORTANT!!!
	// (wrapping behavior)
	for (int i = 0; i < pts[0].size(); i++)
	{
		int id = cds[0].size() - 1 - i;
		uva.push_back({ cds[0][id].x, cds[0][id].y });
		uvb.push_back({ cds[0][id].z, cds[0][id].w });
	}
	for (int i = 0; i < pts[1].size(); i++)
	{
		uva.push_back({ cds[1][i].x, cds[1][i].y });
		uvb.push_back({ cds[1][i].z, cds[1][i].w });
	}
	if (loop)
	{
		output.push_back(pts[0][0]);
		uva.push_back(uva[0]);
		uvb.push_back(uvb[0]);
	}
	return true;
}
