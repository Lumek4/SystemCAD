#include "rough.h"
#include"bicubicSurface.h"
#include"torusGenerator.h"
#include"intersect.h"
#include<fstream>
using namespace DirectX;

XMVECTOR pullBack2(XMVECTOR v, XMMATRIX j, bool& error)
{
	auto jp = XMMatrixTranspose(j) * j;
	jp.r[3].m128_f32[3] = 1;
	jp.r[2].m128_f32[2] = 1;
	XMVECTOR det;
	auto invA = j * XMMatrixInverse(&det, jp);
	auto identity = j * XMMatrixTranspose(invA);
	error = XMVectorGetX(det) == 0 ? true : false;
	return XMVector2Transform(v, XMMatrixTranspose(invA));
}


template<typename TA>
bool convergexy(
	const IntersectData<TA>& dp,
	DirectX::XMVECTOR& point,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR target,
	bool& error,
	const DirectX::XMVECTOR& clampLo,
	const DirectX::XMVECTOR& clampHi,
	int iterations = 150
)
{
	using namespace DirectX;
	XMVECTOR ppos, pdu, pdv;
	XMVECTOR ones{ 1,1,1,1 };
	float lastdist = INFINITY;
	XMVECTOR diff;
	for (int it = 1; ; it++, point += diff
		
		)
	{
		point = XMVectorClamp(point,
			clampLo, clampHi);

		point = XMVectorSelect(point,
			XMVectorMod(point, ones),
			XMVectorGreater(point, ones));
		point = XMVectorSelect(point,
			XMVectorMod(XMVectorMod(point, ones) + ones, ones),
			XMVectorLess(point, XMVectorZero()));

		dp.Point({ XMVectorGetX(point), XMVectorGetY(point) },
			ppos, pdu, pdv);

		float dist = XMVectorGetX(XMVector2Length(ppos - target));
		diff = pullBack2(XMVectorSetZ(ppos - target, 0), XMMATRIX{ -pdu, -pdv, {}, {} }, error);
		if (XMVectorGetX(XMVector2LengthSq(diff))
			< 1e-9)
		{
			position = ppos;
			return true;
		}
		else if (it >= iterations)
			return false;
		if (error)
			return false;
		//lastdist = dist;
	}
}

template<typename T>
float GetHeight(T* object, XMVECTOR cursor)
{
	auto data = IntersectData<T>(object);
	auto bb = data.GetBB();
	if (!AABB2(bb, { cursor, cursor }))
		return -INFINITY;
	XMFLOAT2 point;
	if (std::is_same<T, TorusGenerator>())
		point = nearCursorPoint<T, 256>(data, cursor);
	else
		point = nearCursorPoint<T, 32>(data, cursor);

	XMVECTOR clampLo = {
		data.GetWrapMode().x ? -INFINITY : 0,
		data.GetWrapMode().y ? -INFINITY : 0
	};
	XMVECTOR clampHi = {
		data.GetWrapMode().x ? INFINITY : 1,
		data.GetWrapMode().y ? INFINITY : 1
	};
	XMVECTOR p = { point.x, point.y }, pos;
	bool error = false;
	bool success;
	if(std::is_same<T, TorusGenerator>())
		success = convergexy(data,
		p, pos, cursor, error, clampLo, clampHi, 150);
	else
		success = convergexy(data,
			p, pos, cursor, error, clampLo, clampHi, 7);
	if (success)
		return XMVectorGetZ(pos);
	else
		return -INFINITY;
}
float Raise(XMFLOAT2 position, const std::vector<float>& tex, int resolution, float r, float vertRatio)
{
	int startx = max(0, (position.x - r) * resolution);
	int endx = min(resolution, (position.x + r) * resolution + 1);
	int starty = max(0, (position.y - r) * resolution);
	int endy = min(resolution, (position.y + r) * resolution + 1);

	float maxh = 0;
	for (int y = starty; y < endy; y++)
		for (int x = startx; x < endx; x++)
		{
			float dx = (float)x / resolution - position.x;
			float dy = (float)y / resolution - position.y;
			float dsq = dx * dx + dy * dy;
			float d = r * r - dsq;
			if (d >= 0)
			{

				int id = x + y * resolution;
				float h = tex[id] + uToW(sqrtf(d)-r);
				maxh = fmaxf(maxh, h);
			}
		}
	return uWTouH(wToU(maxh));
}
void Paths::Rough(Entity::Selection& set, Entity* plane, int resolution, float tolerance, std::vector<DirectX::XMFLOAT3>& path)
{
	char cacheFilePath[] = "../RoughTexture_xxxx.b";
	snprintf(cacheFilePath + 16, 5, "%04d", resolution);
	cacheFilePath[20] = '.';
	auto* base = plane->GetComponent<BicubicSurface>();
	if (!base)
		return;
	auto base_data = IntersectData<BicubicSurface>(base);
	XMVECTOR pos, du, dv;
	float maxh = 0;
	std::vector<float> tex;
	auto cacheFile = std::fopen(cacheFilePath, "rb");
	if (!cacheFile)
	{
		for (int y = 0; y < resolution; y++)
		{
			for (int x = 0; x < resolution; x++)
			{
				base_data.Point({ (float)x / resolution, (float)y / resolution },
					pos, du, dv);
				pos = XMVectorSetZ(pos, 5);
				float h = 0;
				for (int i = 0; i < set.size(); i++)
				{
					auto* surf = set[i]->GetComponent<BicubicSurface>();
					auto* toru = set[i]->GetComponent<TorusGenerator>();
					if (surf)
						h = fmaxf(h, GetHeight(surf, pos));
					if (toru)
						h = fmaxf(h, GetHeight(toru, pos));
				}
				maxh = fmaxf(maxh, h);
				tex.push_back(h);
			}
		}
		for (int i = 0; i < tex.size(); i++)
		{
			tex[i] /= 9;
		}
		cacheFile = std::fopen(cacheFilePath, "wb");
		if (cacheFile)
		{
			int count = tex.size();
			std::fwrite(&count, sizeof(count), 1, cacheFile);
			std::fwrite(tex.data(), sizeof(tex[0]), count, cacheFile);
			std::fclose(cacheFile);
		}
		for (int i = 0; i < tex.size(); i++)
		{
			tex[i] *= 9;
		}
	}
	else
	{
		int count;
		std::fread(&count, sizeof(count), 1, cacheFile);
		tex.resize(count);
		std::fread(tex.data(), sizeof(tex[0]), count, cacheFile);
		std::fclose(cacheFile);
		for (int i = 0; i < tex.size(); i++)
		{
			tex[i] *= 9;
			maxh = fmaxf(maxh, tex[i]);
		}
	}
	int headDiameter = 16;
	int baseWidth = 150;
	int baseHeight = 50 - 16;
	float r = mmToU(headDiameter / 2.0f);
	bool backwards = false;
	float width = 2*sqrt(mmToU(tolerance)*r);
	float maxfz = 0;
	for (int y = 0; y < resolution; y++)
	{
		float fy = y * width;
		int startx = backwards ? (resolution - 1) : 0;
		int endx =   backwards ? 0 : (resolution - 1);
		int step =   backwards ? -1 : 1;
		backwards = !backwards;
		for (int x = startx; x * step <= endx * step; x += step)
		{
			float fx = (float)x / resolution;
			float fz = Raise({ fx,fy }, tex, resolution, r, 1) + 1.0f / resolution;
			maxfz = fmaxf(maxfz, fz);
			path.push_back({ fx, fy, fz });
			//path.push_back({ fx, fy, tex[x+y*resolution] + 1 / resolution});
		}
		if (fy > 1)
			break;
	}
	return;
}
