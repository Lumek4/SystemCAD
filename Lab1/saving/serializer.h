#pragma once
#include"../vecmath.h"
#include"../entity.hpp"
#include"../all_components.h"
namespace Deserializer
{
	Entity* Point(DirectX::XMFLOAT3 position, const std::string& name);
}