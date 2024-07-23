#pragma once
#include<DirectXMath.h>
#include<vector>

#define EPS 1e-10
namespace Quaternion
{
	DirectX::XMFLOAT4 Get(DirectX::XMFLOAT3 axis, float angle);
	DirectX::XMFLOAT3 RotateByQuaternion(const DirectX::XMFLOAT3& v,
		const DirectX::XMFLOAT4& q);

	DirectX::XMFLOAT3 ToEulerAngles(DirectX::XMFLOAT4 q);
}



DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& l,
	const DirectX::XMFLOAT3& r);
DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& l,
	const DirectX::XMFLOAT3& r);
DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& l);



template<typename T>
void swap(T& l, T& r)
{
	T tmp = l;
	l = r;
	r = tmp;
}
//template<typename T>
//int defragment(std::vector<T*>& vec)
//{
//	int i = 0;
//	int em = 0;
//	
//	while (true)
//	{
//		while (vec[em] != nullptr)
//		{
//			em++;
//			if (em == vec.size())
//				return em;
//		}
//		if (i < em)
//			i = em;
//		while (vec[i] == nullptr)
//		{
//			i++;
//			if (i == vec.size())
//				return em;
//		}
//		swap(vec[i], vec[em]);
//	}
//	return em;
//}

DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& l,
	float r);
inline DirectX::XMFLOAT3 operator*(float l,
	const DirectX::XMFLOAT3& r) {
	return r * l;
}
#include "vecmath_internals.h"

__VECEQNEQ2DECL(DirectX::XMINT2)
__VECEQNEQ2DECL(DirectX::XMFLOAT2)