#pragma once

#include"entity.hpp"
namespace Paths
{
	void Rough(Entity::Selection& set, Entity* plane,
		int resolution, float tolerance,
		std::vector<DirectX::XMFLOAT3>& path);
}