#include "intersectionCurve.h"
#include"catalogue.hpp"
#include"entity.hpp"
#include"dxDevice.h"
#include"dxStructures.h"
#include"vecmath.h"
using namespace DirectX;
IntersectionCurve::IntersectionCurve(Entity& owner)
	:Component(ComponentConstructorArgs(IntersectionCurve))
{
	//pointSource = &RequireComponent(PointSource);
}
bool inAABB(XMVECTOR& p, XMVECTOR& q, XMVECTOR& a, XMVECTOR& b)
{ 
	XMVECTOR minpq = XMVectorMin(p, q);
	XMVECTOR maxpq = XMVectorMax(p, q);
	XMVECTOR minab = XMVectorMin(a, b);
	XMVECTOR maxab = XMVectorMax(a, b);
	return 
		XMVectorGetX(XMVectorSum(XMVectorLess(maxpq - minab, XMVectorZero())))+
		XMVectorGetX(XMVectorSum(XMVectorLess(maxab - minpq, XMVectorZero()))) <= 0;
}
inline bool collide(XMVECTOR p0, XMVECTOR p1, XMVECTOR p, XMVECTOR pd)
{
	if (!inAABB(p, pd, p0, p1))
		return false;
	XMVECTOR sg =
	{
		XMVectorGetX(XMVector2Cross(p0 - p , p1 - p)),
		XMVectorGetX(XMVector2Cross(p0 - pd, p1 - pd)),
		XMVectorGetX(XMVector2Cross(p - p0, pd - p0)),
		XMVectorGetX(XMVector2Cross(p - p1, pd - p1))
	};
	auto more = XMVectorGreater(sg, { EPS,EPS,EPS,EPS });
	auto less = XMVectorLess(sg, { -EPS,-EPS,-EPS,-EPS });
	sg = XMVectorSelect(more, { 1,1,1,1 }, less);

	return sg.m128_i32[0] != sg.m128_i32[1] && sg.m128_i32[2] != sg.m128_i32[3];
	// punkty po roznych stronach linii
}
void paint(ID3D11Texture2D* tex,
	std::vector<XMFLOAT2>& uv, DxDevice& device, XMINT2 wrapMode)
{
	D3D11_MAPPED_SUBRESOURCE m;
	device.context()->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	constexpr int maxres = 128;
	auto data = (uint8_t*)m.pData;

	XMVECTOR directions[4] = {
		{1.0f / maxres, 0},
		{0, 1.0f / maxres},
		{-1.0f / maxres, 0},
		{0, -1.0f / maxres}
	};
	XMINT2 i_directions[4] = {
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1}
	};
	std::vector<XMINT2> flood{};
	int c_region = 0;

	for (int y = 0; y < maxres; y++) for (int x = 0; x < maxres; x++)
		data[x * 4 + y * m.RowPitch] = 0;

	long long painted = 0;
	for (int yy = 0; yy < maxres; yy++) for (int xx = 0; xx < maxres; xx++)
	{
		if (data[xx * 4 + yy * m.RowPitch] == 0)
		{
			flood.push_back({ xx,yy });
			data[xx * 4 + yy * m.RowPitch] = ++c_region;
			painted++;
			if (c_region >= 128)
			{
				yy = maxres; xx = maxres;
				break;
			}
		}
		while (!flood.empty()/* && painted < maxres*maxres/4.5*/)
		{
			XMINT2 ip = flood.front(); flood.erase(flood.begin());
			XMVECTOR p = { (0.5f + ip.x) / maxres, (0.5f + ip.y) / maxres };
			for (int j = 0; j < 4; j++)
			{
				XMINT2 ipd = { ip.x + i_directions[j].x, ip.y + i_directions[j].y };
				if ((ipd.x < 0 || ipd.x >= maxres) && !wrapMode.x ||
					(ipd.y < 0 || ipd.y >= maxres) && !wrapMode.y)
					continue;
				XMINT2 wrapped = {};
				if (ipd.x >= maxres) wrapped.x = -maxres;
				else if (ipd.x < 0) wrapped.x = maxres;
				if (ipd.y >= maxres) wrapped.y = -maxres;
				else if (ipd.y < 0) wrapped.y = maxres;
				ipd.x += wrapped.x;
				ipd.y += wrapped.y;

				if (data[ipd.x * 4 + ipd.y * m.RowPitch] != 0)
					continue;
				XMVECTOR pd = p + directions[j];
				XMVECTOR p0, p1 = XMLoadFloat2(&uv[0]);
				bool blocked = false;
				for (int i = 1; i < uv.size(); i++)
				{
					p0 = p1; p1 = XMLoadFloat2(&uv[i]);
					if (collide(p0, p1, p, pd) || ((wrapped.x || wrapped.y) &&
						collide(p0,p1,
							p  + directions[0] * wrapped.x + directions[1] * wrapped.y,
							pd + directions[0] * wrapped.x + directions[1] * wrapped.y)))
					{
						blocked = true;
						break;
					}
				}
				if(!blocked)
				{
					flood.push_back(ipd);
					data[ipd.x * 4 + ipd.y * m.RowPitch] = c_region;
					painted++;
				}
			}
		}
	}
	XMINT3 colors[] = {
		{0, 0, 0},
		{255, 0, 63},
		{63, 255, 0},
		{0, 63, 255},
		{127, 127, 0},
		{0, 127, 127},
		{127, 0, 127},
		{255, 63, 0},
		{0, 255, 63},
		{63, 0, 255},
		{63, 63, 127},
		{127, 63, 63},
		{63, 127, 63},
	};
	for (int y = 0; y < maxres; y++) for (int x = 0; x < maxres; x++)
	{
		int color = data[(x * 4 + y * m.RowPitch)];
		if (color > (sizeof(colors) / sizeof(XMINT3)))
			color = 0;

		data[(x * 4 + y * m.RowPitch) + 0] = colors[color].x;
		data[(x * 4 + y * m.RowPitch) + 1] = colors[color].y;
		data[(x * 4 + y * m.RowPitch) + 2] = colors[color].z;
		data[(x * 4 + y * m.RowPitch) + 3] = 255;
	}
	/*for (int i = 1; i < uv.size(); i++)
	{
		int x = uv[i-1].x * maxres, xt = uv[i].x * maxres;
		int y = uv[i-1].y * maxres, yt = uv[i].y * maxres;
		int dx = xt - x;
		int dy = yt - y;
		bool y_wards = fabsf(dx) < fabsf(dy);
		if (y_wards)
		{
			std::swap(x, y);
			std::swap(dx, dy);
			std::swap(xt, yt);
		}
		if (dx < 0)
		{
			std::swap(x, xt);
			std::swap(y, yt);
			dx = -dx;
			dy = -dy;
		}
		int sy = dy < 0 ? -1 : 1;
		dy *= sy;

		int D = 2 * dy - dx;
		for(x; x<xt; x++)
		{
			if (y_wards)
			{
				data[(y * 4 + x * m.RowPitch) + 0] = 255;
				data[(y * 4 + x * m.RowPitch) + 1] = 255;
				data[(y * 4 + x * m.RowPitch) + 2] = 255;
				data[(y * 4 + x * m.RowPitch) + 3] = 255;
			}
			else
			{
				data[(x * 4 + y * m.RowPitch) + 0] = 255;
				data[(x * 4 + y * m.RowPitch) + 1] = 255;
				data[(x * 4 + y * m.RowPitch) + 2] = 255;
				data[(x * 4 + y * m.RowPitch) + 3] = 255;
			}
			if (D > 0)
			{
				y += sy;
				D += 2 * (dy - dx);
			}
			else
			{
				D += 2 * dy;
			}
		}
	}*/
	device.context()->Unmap(tex, 0);
}
void IntersectionCurve::InitTextures(DxDevice& device, int resolution)
{
	auto desc = Texture2DDescription::CPUTextureDescription(resolution, resolution);
	texA = device.CreateTexture(desc);
	texB = device.CreateTexture(desc);
	texvA = device.CreateShaderResourceView(texA);
	texvB = device.CreateShaderResourceView(texB);
	paint(texA.get(), pointsA, device, wrapModeA);
	paint(texB.get(), pointsB, device, wrapModeB);
}
