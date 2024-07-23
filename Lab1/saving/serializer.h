#pragma once
#include"../vecmath.h"
#include"../entity.hpp"
namespace Serializer
{
	void Torus(Entity* e,
		std::string& name,
		DirectX::XMFLOAT2& radii,
		DirectX::XMFLOAT3& position,
		DirectX::XMFLOAT3& rotation,
		DirectX::XMFLOAT3& scale,
		DirectX::XMINT2& samples);
	void Curve(Entity* e,
		std::string& name,
		Entity::Selection& controlPoints);

	void Surface(Entity* e,
		std::string& name,
		std::vector<std::string_view>& segmentNames,
		Entity::Selection& controlPoints,
		DirectX::XMINT2& parameterWrapped,
		DirectX::XMINT2& size);
}