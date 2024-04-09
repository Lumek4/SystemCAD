#include "sceneCursor.h"
#include"imgui/imgui.h"
using namespace DirectX;

SceneCursor SceneCursor::instance{};

void SceneCursor::Correct(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj, const DirectX::XMFLOAT4X4& invview, const DirectX::XMFLOAT4X4& invproj)
{
	
	auto size = ImGui::GetIO().DisplaySize;
	if (sChanged && !isnan(screen.x) && !isnan(screen.y))
	{
		XMFLOAT4 v = { world.x, world.y, world.z, 1 };

		XMStoreFloat4(&v,
			XMVector4Transform(
				XMVector4Transform(
					XMLoadFloat4(&v),
					XMLoadFloat4x4(&view)
				),
				XMLoadFloat4x4(&proj)
			)
		);

		XMFLOAT4 norm = { 2 * screen.x / size.x - 1, 2 * screen.y / size.y - 1, v.z / v.w, 1 };
		XMStoreFloat4(&v,
			XMVector4Transform(
				XMVector4Transform(
					XMLoadFloat4(&norm),
					XMLoadFloat4x4(&invproj)
				),
				XMLoadFloat4x4(&invview)
			)
		);
		world = { v.x / v.w,v.y / v.w,v.z / v.w };
		sChanged = false;
	}

	{
		XMFLOAT4 v = { world.x, world.y, world.z, 1 };

		XMStoreFloat4(&v,
			XMVector4Transform(
				XMVector4Transform(
					XMLoadFloat4(&v),
					XMLoadFloat4x4(&view)
				),
				XMLoadFloat4x4(&proj)
			)
		);
		screen = {
			(0.5f * v.x / v.w + 0.5f) * size.x,
			(0.5f * v.y / v.w + 0.5f) * size.y,
		};
		if (screen.x<0 || screen.x > size.x ||
			screen.y<0 || screen.y > size.y)
			screen.x = screen.y = NAN;
	}
}
