#pragma once
#include<DirectXMath.h>
class MyMat : public DirectX::XMMATRIX
{
public:
	MyMat();
	MyMat(const float* values);
	MyMat(const MyMat& m);
	MyMat(const DirectX::XMMATRIX& m);
	MyMat& operator=(const MyMat& m);


	static MyMat Identity();
	static MyMat Perspective(float fov, float aspect, float n, float f,
		float skew = 0, float convergence = 1);
	//static MyMat InvPerspective(float fov, float aspect, float n, float f);
private:
	static const float identityInit[16];
};

class MyMRot : public MyMat
{
public:
	MyMRot(DirectX::XMFLOAT4 quaternion);
	MyMRot(DirectX::XMFLOAT3 axes);
	MyMRot Invert();
};
class MyMTrans : public MyMat
{
public:
	MyMTrans(DirectX::XMFLOAT3 offset);
	MyMTrans Invert();
};
class MyMScale : public MyMat
{
public:
	MyMScale(DirectX::XMFLOAT3 factors);
	MyMScale Invert();
};
