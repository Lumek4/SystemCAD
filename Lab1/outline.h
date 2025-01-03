#pragma once
#include"entity.hpp"
namespace Paths
{
	void OutlineSet(Entity::Selection& set, Entity* plane, DirectX::XMFLOAT3 cursor,
		float precision,
		std::vector<DirectX::XMFLOAT2>& outline,
		std::vector<DirectX::XMFLOAT2>& fill);
}