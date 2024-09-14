#pragma once
#include<DirectXMath.h>
namespace ColorPalette
{
	static constexpr DirectX::XMFLOAT4 Darken(const DirectX::XMFLOAT4& color, float darken)
	{
		return {
			color.x * (1 - darken),
			color.y * (1 - darken),
			color.z * (1 - darken),
			color.w
		};
	}
	static constexpr DirectX::XMFLOAT4 Lighten(const DirectX::XMFLOAT4& color, float lighten)
	{
		return {
			color.x * (1 - lighten) + lighten,
			color.y * (1 - lighten) + lighten,
			color.z * (1 - lighten) + lighten,
			color.w
		};
	}
	static constexpr DirectX::XMFLOAT4 white{ 1,1,1,1 };
	static constexpr DirectX::XMFLOAT4 gray{ 0.5f,0.5f,0.5f,1 };
	static constexpr DirectX::XMFLOAT4 selectYellow{ 0.9f, 0.7f, 0.2f, 1 };
	static constexpr DirectX::XMFLOAT4 highlightOrange{ 1, 0.6f, 0.0f, 1 };
	static constexpr DirectX::XMFLOAT4 vEditMagenta{ 1.0f, 0.0f, 0.9f, 1 };
	static constexpr DirectX::XMFLOAT4 virtualPurple{ 0.5f, 0.2f, 0.5f, 1 };

	static constexpr DirectX::XMFLOAT4 trims[] = {
		{0, 0, 0, 1},
		{1, 0, 0.25f, 1},
		{0.25f, 1, 0, 1},
		{0, 0.25f, 1, 1},
		{0.75f, 0.75f, 0, 1},
		{0, 0.75f, 0.75f, 1},
		{0.75f, 0, 0.75f, 1},
		{1, 0.25f, 0, 1},
		{0, 1, 0.25f, 1},
		{0.25f, 0, 1, 1},
		{0.25f, 0.25f, 0.75f, 1},
		{0.75f, 0.25f, 0.25f, 1},
		{0.25f, 0.75f, 0.25f, 1},
	};
}