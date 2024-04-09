#include "myMat.h"
#include<d3d11.h>
#include<memory>
using namespace DirectX;

const float MyMat::identityInit[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
};

MyMat::MyMat()
{
    ZeroMemory(this, sizeof(MyMat));
}

MyMat::MyMat(const float* values)
    :XMMATRIX(values)
{}

MyMat::MyMat(const MyMat & m)
{
    CopyMemory(this, &m, sizeof(MyMat));
}

MyMat::MyMat(const DirectX::XMMATRIX& m)
{
    CopyMemory(this, &m, sizeof(XMMATRIX));
}

MyMat& MyMat::operator=(const MyMat& m)
{
    CopyMemory(this, &m, sizeof(MyMat));
    return *this;
}


MyMat MyMat::Identity()
{
    return MyMat(MyMat::identityInit);
}

MyMat MyMat::Perspective(float fov, float aspect, float n, float f)
{
    return MyMat({
        1/tanf(fov*0.5f)/aspect, 0, 0, 0,
        0, 1/tanf(fov*0.5f), 0, 0,
        0, 0, (f+n)/(f-n), 1,
        0, 0, -2 * f * n / (f - n), 0
        });
}
MyMat MyMat::InvPerspective(float fov, float aspect, float n, float f)
{
    float v = (f * f - n * n) / (2 * f * n);
    return MyMat({
        aspect * tanf(fov * 0.5f), 0, 0, 0,
        0, tanf(fov * 0.5f), 0, 0,
        0, 0, 0, -v,
        0, 0,f-n, v
        });
}

MyMRot::MyMRot(DirectX::XMFLOAT4 quaternion)
{
    float r = quaternion.w, i = quaternion.x, j = quaternion.y, k = quaternion.z;
    *((MyMat*)this) = XMMatrixSet(
        1-2*(j*j+k*k), 2*(i*j-k*r)  , 2*(i*k+j*r)  , 0,
        2*(i*j+k*r)  , 1-2*(i*i+k*k), 2*(j*k-i*r)  , 0,
        2*(i*k-j*r)  , 2*(j*k+i*r)  , 1-2*(i*i+j*j), 0,
        0, 0, 0, 1
    );
}

MyMRot::MyMRot(DirectX::XMFLOAT3 axes)
{

    float x = axes.x, y = axes.y, z = axes.z;
    float rotx[] = {
        cos(x), 0, sin(x), 0,
        0, 1, 0, 0,
        -sin(x), 0, cos(x), 0,
        0, 0, 0, 1

    };
    float roty[] = {
        1, 0, 0, 0,
        0, cos(y), sin(y), 0,
        0, -sin(y), cos(y), 0,
        0, 0, 0, 1
    };
    float rotz[] = {
        cos(z), sin(z), 0, 0,
        -sin(z), cos(z), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1

    };
    (*(MyMat*)this) = MyMat(rotz)* MyMat(roty)* MyMat(rotx);
}


MyMRot MyMRot::Invert()
{
    auto mat = XMMatrixTranspose(*this);
    return *(MyMRot*)&mat;
}

MyMTrans::MyMTrans(DirectX::XMFLOAT3 offset)
    :MyMat({
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        offset.x, offset.y, offset.z, 1
        })
{}

MyMTrans MyMTrans::Invert()
{
    auto mat = (2 * XMMatrixIdentity()) - *this;
    return *(MyMTrans*) &mat;
}

MyMScale::MyMScale(DirectX::XMFLOAT3 factors)
    :MyMat({
        factors.x, 0, 0, 0,
        0, factors.y, 0, 0,
        0, 0, factors.z, 0,
        0, 0, 0, 1
        })
{}

MyMScale MyMScale::Invert()
{
    return MyMScale({
        1 / ::XMVectorGetX(r[0]),
        1 / ::XMVectorGetY(r[1]),
        1 / ::XMVectorGetZ(r[2])
        });
}
