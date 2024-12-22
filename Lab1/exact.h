#pragma once

#include"entity.hpp"
namespace Paths
{
	void Exact(Entity::Selection& set,
		float precision,
		std::vector<DirectX::XMFLOAT3>& path);
}