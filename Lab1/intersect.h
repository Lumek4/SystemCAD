#pragma once
#include"bicubicSegment.h"

bool intersect(BicubicSegment* a, BicubicSegment* b,
	DirectX::XMFLOAT3 cursor, float precision,
	std::vector<DirectX::XMFLOAT3>& output,
	std::vector<DirectX::XMFLOAT2>& uva,
	std::vector<DirectX::XMFLOAT2>& uvb,
	bool& loop);