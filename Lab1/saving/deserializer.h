#pragma once
#include"../vecmath.h"
#include"../entity.hpp"
namespace Deserializer
{
	Entity* Point(const std::string& name,
		DirectX::XMFLOAT3 position);

	Entity* Torus(const std::string& name,
		DirectX::XMFLOAT2 radii,
		DirectX::XMFLOAT3 position,
		DirectX::XMFLOAT3 rotation,
		DirectX::XMFLOAT3 scale,
		DirectX::XMINT2 samples);
	Entity* BezierC0(const std::string& name,
		const Entity::Selection& controlPoints);
	Entity* BezierC2(const std::string& name,
		const Entity::Selection& deBoorPoints);
	Entity* InterpolatedC2(const std::string& name,
		const Entity::Selection& controlPoints);

	Entity* BezierSurfaceC0(Entity::Selection& outSegments,
		const std::string& name,
		const std::vector<std::string*>& segmentNames,
		const Entity::Selection& controlPoints,
		DirectX::XMINT2 parameterWrapped,
		DirectX::XMINT2 size);
	Entity* BezierSurfaceC2(Entity::Selection& outSegments,
		const std::string& name,
		const std::vector<std::string*>& segmentNames,
		const Entity::Selection& controlPoints,
		DirectX::XMINT2 parameterWrapped,
		DirectX::XMINT2 size);
}