#include "vecmath.h"
using namespace DirectX;
DirectX::XMFLOAT4 Quaternion::Get(DirectX::XMFLOAT3 axis, float angle)
{
    float sin = sinf(angle / 2);
    return { axis.x * sin, axis.y * sin, axis.z * sin, cosf(angle / 2) };
}
DirectX::XMFLOAT3 Quaternion::RotateByQuaternion(const DirectX::XMFLOAT3& v, const DirectX::XMFLOAT4& q)
{
    XMFLOAT4 v4 = { v.x, v.y, v.z, 0 };
    XMVECTOR qq = XMLoadFloat4(&q), vv = XMLoadFloat4(&v4);
    
    XMFLOAT3 res;
    XMStoreFloat3(&res,
        XMQuaternionMultiply(
            XMQuaternionMultiply(qq, vv),
            XMQuaternionConjugate(qq)
        )
    );
    return res;
}


DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& l, const DirectX::XMFLOAT3& r)
{
    return DirectX::XMFLOAT3(l.x+r.x,l.y+r.y,l.z+r.z);
}

DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& l, const DirectX::XMFLOAT3& r)
{
    return DirectX::XMFLOAT3(l.x-r.x,l.y-r.y,l.z-r.z);
}

DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& l)
{
    return DirectX::XMFLOAT3(-l.x,-l.y,-l.z);
}

DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& l, float r)
{
    return DirectX::XMFLOAT3(l.x*r,l.y*r,l.z*r);
}