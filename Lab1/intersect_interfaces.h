#pragma once
#include<DirectXMath.h>
#include<array>
#include<vector>
#include "exceptions.h"

template<typename T>
class IntersectData;

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
	const DirectX::XMINT2& GetWrapMode() const { return wrapMode; }
private:
	DirectX::XMINT2 wrapMode = { 0,0 };
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
	const DirectX::XMINT2& GetWrapMode() const { return wrapMode; }
private:
	DirectX::XMINT2 wrapMode = { 1,1 };
	BoundingBox bb;
	DirectX::XMFLOAT2 radii;
	DirectX::XMMATRIX transform;
};

template<typename T>
class Inflation : public IntersectData<T>
{
public:
	const float R;
	bool specialTreatment = false;
	Inflation(T* object, float R)
		:IntersectData<T>(object), R(R)
	{}
};

template<typename T>
class IntersectData<Inflation<T>>
{
private:
	Inflation<T>* data;
public:
	IntersectData<Inflation<T>>(Inflation<T>* object);
	inline void Point(DirectX::XMFLOAT2 uv,
		DirectX::XMVECTOR& position,
		DirectX::XMVECTOR& du,
		DirectX::XMVECTOR& dv) const;
	const BoundingBox& GetBB() const { return bb; }
	const DirectX::XMINT2& GetWrapMode() const { return data->GetWrapMode(); }
private:
	BoundingBox bb;
};

template<typename T>
IntersectData<Inflation<T>>::IntersectData<Inflation<T>>
(Inflation<T>* object)
{
	data = object;
	bb = object->GetBB();
}
template<typename T>
inline void IntersectData<Inflation<T>>::Point(DirectX::XMFLOAT2 uv,
	DirectX::XMVECTOR& position,
	DirectX::XMVECTOR& du,
	DirectX::XMVECTOR& dv) const
{
	using namespace DirectX;
	data->Point(uv, position, du, dv);
	position = position + XMVector3Normalize(XMVector3Cross(du, dv))*data->R;
	float h = 1e-5;
	if (!data->specialTreatment && uv.x + h <= 1)
	{
		XMVECTOR pu, duu, dvu;
		data->Point({ uv.x + h, uv.y }, pu, duu, dvu);
		pu = pu + XMVector3Normalize(XMVector3Cross(duu, dvu)) * data->R;
		du = (pu - position) / h;
	}

	if (!data->specialTreatment && uv.y + h <= 1)
	{
		XMVECTOR pv, duv, dvv;
		data->Point({ uv.x, uv.y + h }, pv, duv, dvv);
		pv = pv + XMVector3Normalize(XMVector3Cross(duv, dvv)) * data->R;
		dv = (pv - position) / h;
	}
}