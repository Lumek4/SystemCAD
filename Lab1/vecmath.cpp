#include "vecmath.h"
#include<numbers>
#include<cmath>
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
DirectX::XMFLOAT3 Quaternion::ToEulerAngles(DirectX::XMFLOAT4 q) {
    DirectX::XMFLOAT3 angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.z * q.y);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.z * q.z);
    angles.x = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (z-axis rotation)
    double sinp = std::sqrt(1 + 2 * (q.w * q.z - q.x * q.y));
    double cosp = std::sqrt(1 - 2 * (q.w * q.z - q.x * q.y));
    angles.y = 2 * std::atan2(sinp, cosp) - XM_PI / 2;

    // yaw (y-axis rotation)
    double siny_cosp = 2 * (q.w * q.y + q.x * q.z);
    double cosy_cosp = 1 - 2 * (q.z * q.z + q.y * q.y);
    angles.z = std::atan2(siny_cosp, cosy_cosp);

    return angles;
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

__VECEQNEQ2(DirectX::XMINT2)
__VECEQNEQ2(DirectX::XMFLOAT2)

float vecmath::screenRay(DirectX::XMVECTOR point,
    DirectX::XMVECTOR ray, DirectX::XMMATRIX viewproj,
    float radius, float aspect)
{
    point = XMVector4Transform(
        point, viewproj
    );
    point /= XMVectorGetW(point);
    if (XMVectorGetX(XMVector2LengthSq((ray - point) * XMVECTOR{ 1, aspect })) <= radius * radius)
        return XMVectorGetZ(point);
    else
        return NAN;
}

bool vecmath::screenBox(DirectX::XMVECTOR point, DirectX::XMVECTOR lo, DirectX::XMVECTOR hi, DirectX::XMMATRIX viewproj)
{
    point = XMVector4Transform(
        point, viewproj
    );
    point /= XMVectorGetW(point);
    return XMVector2Less(point,hi)  && XMVector2Greater(point,lo);
}

float vecmath::clamp01(float v)
{
    return v<0?0:(v>1?1:v);
}

float vecmath::length(DirectX::XMFLOAT3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vecmath::lengthSq(DirectX::XMFLOAT3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

DirectX::XMFLOAT3 vecmath::argwiseMul(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b)
{
    return DirectX::XMFLOAT3(a.x*b.x, a.y*b.y, a.z*b.z);
}

DirectX::XMFLOAT3& vecmath::get(DirectX::XMFLOAT3X3& m, int k)
{
    assert(k < 3 && k >= 0);
    return ((DirectX::XMFLOAT3*)&m)[k];
}
