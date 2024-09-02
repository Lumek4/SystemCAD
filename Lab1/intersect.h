#pragma once
#include"bicubicSegment.h"

bool intersect(BicubicSegment* a, BicubicSegment* b,
	std::vector<DirectX::XMFLOAT3>& output, DirectX::XMFLOAT3 cursor);