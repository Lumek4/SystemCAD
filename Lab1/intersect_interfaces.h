#pragma once
#include<DirectXMath.h>
#include<array>
#include<vector>

template<typename T>
class IntersectData {};

struct BoundingBox
{
	DirectX::XMVECTOR lo;
	DirectX::XMVECTOR hi;
};

class BicubicSegment;
template<>
class IntersectData<BicubicSegment>
{
public:
	IntersectData<BicubicSegment>(BicubicSegment* object);
	void Point(DirectX::XMFLOAT2 uv,
		DirectX::XMVECTOR& position,
		DirectX::XMVECTOR& du,
		DirectX::XMVECTOR& dv) const;
	const BoundingBox& GetBB() const { return bb; }
	const DirectX::XMINT2& GetWrapMode() const { return { 0,0 }; }
private:
	BoundingBox bb;
	std::array<DirectX::XMFLOAT3, 16> cpoints;
};

class BicubicSurface;
template<>
class IntersectData<BicubicSurface>
{
public:
	IntersectData<BicubicSurface>(BicubicSurface* object);
	void Point(DirectX::XMFLOAT2 uv,
		DirectX::XMVECTOR& position,
		DirectX::XMVECTOR& du,
		DirectX::XMVECTOR& dv) const;
	const BoundingBox& GetBB() const { return bb; }
	const DirectX::XMINT2& GetWrapMode() const { return wrapMode; }
private:
	BoundingBox bb;
	DirectX::XMINT2 division;
	DirectX::XMINT2 wrapMode;
	std::vector<std::array<DirectX::XMFLOAT3, 16>> cpoints;
};

class TorusGenerator;
template<>
class IntersectData<TorusGenerator>
{
public:
	IntersectData<TorusGenerator>(TorusGenerator* object);
	void Point(DirectX::XMFLOAT2 uv,
		DirectX::XMVECTOR& position,
		DirectX::XMVECTOR& du,
		DirectX::XMVECTOR& dv) const;
	const BoundingBox& GetBB() const { return bb; }
	const DirectX::XMINT2& GetWrapMode() const { return { 1,1 }; }
private:
	BoundingBox bb;
	DirectX::XMFLOAT2 radii;
	DirectX::XMMATRIX transform;
};