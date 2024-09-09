#pragma once
#include<DirectXMath.h>
template<typename T>
class IntersectData
{
};

class BicubicSegment;
template<>
class IntersectData<BicubicSegment>
{
public:
	IntersectData<BicubicSegment>(BicubicSegment& object);
	void point(DirectX::XMFLOAT2 uv,
		DirectX::XMVECTOR& position,
		DirectX::XMVECTOR& du,
		DirectX::XMVECTOR& dv);
private:
	std::array<DirectX::XMFLOAT3, 16> cpoints;
};


