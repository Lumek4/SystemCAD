#pragma once
#include"sceneCursor.h"
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
template<typename T, int maxres>
DirectX::XMFLOAT2 nearCursorPoint(
	const IntersectData<T>& data,
	const DirectX::XMVECTOR& cur)
{
	using namespace DirectX;
	float mindist = INFINITY;
	XMINT2 mincoord;
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
static DirectX::XMVECTOR pullBack(
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
	DirectX::XMVECTOR& position,
	bool& error, bool self,
	const DirectX::XMVECTOR& clampLo,
	const DirectX::XMVECTOR& clampHi,
	int iterations = 150
)
{
	using namespace DirectX;
	XMVECTOR apos, adu, adv;
	XMVECTOR bpos, bdu, bdv;
	XMVECTOR pt1{}, pt2{};
	XMVECTOR ones{ 1,1,1,1 };
	for (int it = 1; ; it++, point +=
		pullBack(bpos - apos, { adu, adv, -bdu, -bdv }, error)
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
		
		point = XMVectorSelect(point,
			XMVectorMod(point, ones),
			XMVectorGreater(point, ones));
		point = XMVectorSelect(point,
			XMVectorMod(XMVectorMod(point, ones) + ones,ones),
			XMVectorLess(point, XMVectorZero()));
		da.Point({ XMVectorGetX(point), XMVectorGetY(point) },
			apos, adu, adv);
		db.Point({ XMVectorGetZ(point), XMVectorGetW(point) },
			bpos, bdu, bdv);


		float dist = XMVectorGetX(XMVector3Length(bpos - apos));
		if (dist < 1e-3)
		{
			if (self)
			{
				float d = uvDist(point, adu, adv);
				if (d < 1e-3)
					return false;
			}
			position = apos;
			return true;
		}
		else if (it >= iterations)
			return false;
	}
}

template<typename TA, typename TB>
bool intersect(
	const IntersectData<TA>& da,
	const IntersectData<TB>& db,
	bool self,
	DirectX::XMFLOAT3 cursor, float precision,
	std::vector<DirectX::XMFLOAT3>& output,
	std::vector<DirectX::XMFLOAT2>& uva,
	std::vector<DirectX::XMFLOAT2>& uvb,
	bool& loop)
{
	using namespace DirectX;
	loop = false;
	auto wrap = XMConvertVectorFloatToInt(XMVECTOR{
		-(float)da.GetWrapMode().x, -(float)da.GetWrapMode().y,
		-(float)db.GetWrapMode().x, -(float)db.GetWrapMode().y
		},0);
	if (!AABB(da.GetBB(), db.GetBB()))
		return false;

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
	int it = 0;

	float dist = INFINITY;
	bool error = false;
	XMVECTOR vcursor = XMLoadFloat3(&cursor);
	XMVECTOR pos;
	XMVECTOR startCoord, coord;
	srand(2137);
	for (int i = 0; i < 10000; i++)
	{
		coord = {
			rand() * 1.0f / RAND_MAX,
			rand() * 1.0f / RAND_MAX,
			rand() * 1.0f / RAND_MAX,
			rand() * 1.0f / RAND_MAX
		};
		if (converge(da, db, coord, pos, error, self, clampLo, clampHi))
		{
			float d = XMVectorGetX(XMVector3Length(pos - vcursor));
			if (d < dist)
			{
				dist = d;
				startCoord = coord;
			}
		}
	}
	if(isinf(dist))
		return false;


	XMVECTOR apos, adu, adv;
	XMVECTOR bpos, bdu, bdv;
	da.Point({ XMVectorGetX(startCoord), XMVectorGetY(startCoord) },
		apos, adu, adv);
	db.Point({ XMVectorGetZ(startCoord), XMVectorGetW(startCoord) },
		bpos, bdu, bdv);
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
		for (int it = 1; it < 100000; it++)
		{
			auto tangent = sgn * XMVector3Normalize(XMVector3Cross(
				XMVector3Cross(adu, adv),
				XMVector3Cross(bdu, bdv)
			)) * precision;
			bool error;
			coord = coord + pullBack(tangent, { adu, adv, bdu, bdv }, error);
			if(error)
				return false;
			if (!converge(da, db, coord, pos, error, self, clampLo, clampHi, 10))
				return false;
			if (error)
				return false;

			da.Point({ XMVectorGetX(coord), XMVectorGetY(coord) },
				apos, adu, adv);
			db.Point({ XMVectorGetZ(coord), XMVectorGetW(coord) },
				bpos, bdu, bdv);

			pts[(sgn + 1) / 2].push_back({});
			XMStoreFloat3(&pts[(sgn + 1) / 2].back(), pos);
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
				&& XMVectorGetX(XMVector4LengthSq(
					coord - XMLoadFloat4(&cds[0][0])))
				+ XMVectorGetX(XMVector4LengthSq(
					coord - XMLoadFloat4(&cds[0][1]))) <
				XMVectorGetX(XMVector4LengthSq(
					XMLoadFloat4(&cds[0][0]) - XMLoadFloat4(&cds[0][1])))
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

	uva.push_back({ cds[0][cds[0].size()-1].x, cds[0][cds[0].size()-1].y });
	uvb.push_back({ cds[0][cds[0].size()-1].z, cds[0][cds[0].size()-1].w });
	for (int i = 1; i < pts[0].size(); i++)
	{
		int id = cds[0].size() - 1 - i;
		auto& p0 = cds[0][id+1];
		auto& p1 = cds[0][id+0];

		auto v1 = XMLoadFloat4(&p1);
		auto v0 = XMLoadFloat4(&p0);
		XMVECTOR wrapMod =
			XMVectorSelect({ -1,-1,-1,-1 }, { 0,0,0,0 },
				XMVectorGreater(v1 - v0, { 0.5f,0.5f,0.5f,0.5f })) +
			XMVectorSelect({ 1,1,1,1 }, { 0,0,0,0 },
				XMVectorGreater(v0 - v1, { 0.5f,0.5f,0.5f,0.5f }));

		if (XMVectorGetX(XMVector4Dot(wrapMod, wrapMod)))
		{
			v1 -= wrapMod;
			v0 += wrapMod;
			uva.push_back({ XMVectorGetX(v1) , XMVectorGetY(v1) });
			uva.push_back({ XMVectorGetX(v0) , XMVectorGetY(v0) });
			uvb.push_back({ XMVectorGetZ(v1) , XMVectorGetW(v1) });
			uvb.push_back({ XMVectorGetZ(v0) , XMVectorGetW(v0) });
		}

		uva.push_back({ p1.x, p1.y });
		uvb.push_back({ p1.z, p1.w });
	}if (!cds[1].empty())
	{
		uva.push_back({ cds[1][0].x,cds[1][0].y });
		uvb.push_back({ cds[1][0].z,cds[1][0].w });
	}
	for (int i = 1; i < pts[1].size(); i++)
	{
		auto& p0 = cds[1][i - 1];
		auto& p1 = cds[1][i - 0];

		auto v1 = XMLoadFloat4(&p1);
		auto v0 = XMLoadFloat4(&p0);
		XMVECTOR wrapMod =
			XMVectorSelect({ -1,-1,-1,-1 }, { 0,0,0,0 },
				XMVectorGreater(v1 - v0, { 0.5f,0.5f,0.5f,0.5f })) +
			XMVectorSelect({ 1,1,1,1 }, { 0,0,0,0 },
				XMVectorGreater(v0 - v1, { 0.5f,0.5f,0.5f,0.5f }));

		if (XMVectorGetX(XMVector4Dot(wrapMod, wrapMod)))
		{
			v1 -= wrapMod;
			v0 += wrapMod;
			uva.push_back({ XMVectorGetX(v1) , XMVectorGetY(v1) });
			uva.push_back({ XMVectorGetX(v0) , XMVectorGetY(v0) });
			uvb.push_back({ XMVectorGetZ(v1) , XMVectorGetW(v1) });
			uvb.push_back({ XMVectorGetZ(v0) , XMVectorGetW(v0) });
		}

		uva.push_back({ p1.x, p1.y });
		uvb.push_back({ p1.z, p1.w });
	}
	if (loop)
	{
		output.push_back(pts[0][0]);
		uva.push_back({ cds[0][cds[0].size()-1].x, cds[0][cds[0].size()-1].y });
		uvb.push_back({ cds[0][cds[0].size()-1].z, cds[0][cds[0].size()-1].w });
	}
	return true;
}

template<typename TA, typename TB>
bool intersect(TA* a, TB* b,
	DirectX::XMFLOAT3 cursor, float precision,
	std::vector<DirectX::XMFLOAT3>& output,
	std::vector<DirectX::XMFLOAT2>& uva,
	std::vector<DirectX::XMFLOAT2>& uvb,
	bool& loop)
{
	auto da = IntersectData<TA>(a);
	auto db = IntersectData<TB>(b);
	bool self = (void*)a == (void*)b;
	return intersect(da, db, self,
		cursor, precision,
		output, uva, uvb, loop);
}

template<typename TA, typename TB>
void multiIntersect(
	std::vector<TA>& as, std::vector<TB>& bs, float precision,
	std::vector<std::vector<DirectX::XMFLOAT3>>& pts,
	std::vector<std::vector<DirectX::XMFLOAT2>>& uva,
	std::vector<std::vector<DirectX::XMFLOAT2>>& uvb,
	std::vector<Entity*>& a,
	std::vector<Entity*>& b)
{
	using namespace DirectX;
	bool loop = false;
	std::vector<DirectX::XMFLOAT3> lpts;
	std::vector<DirectX::XMFLOAT2> luva;
	std::vector<DirectX::XMFLOAT2> luvb;
	int s = ((void*)&as == (void*)&bs) ? 1 : 0;
	for (int i = 0; i < as.size(); i++)
		for (int j = i*s; j < bs.size(); j++)
		{
			if (intersect(as[i], bs[j],
				SceneCursor::instance.GetWorld(), precision,
				lpts, luva, luvb, loop))
			{
				a.push_back(&as[i]->owner);
				b.push_back(&bs[j]->owner);
				pts.push_back(lpts);
				uva.push_back(luva);
				uvb.push_back(luvb);
			}
			lpts.clear();
			luva.clear();
			luvb.clear();
		}
}