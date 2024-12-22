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

float vecmath::length(DirectX::XMFLOAT2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vecmath::lengthSq(DirectX::XMFLOAT2 v)
{
    return v.x * v.x + v.y * v.y;
}

float vecmath::triArea(DirectX::XMFLOAT2 a, DirectX::XMFLOAT2 b, DirectX::XMFLOAT2 c)
{
    XMFLOAT2 da = { a.x - c.x, a.y - c.y };
    XMFLOAT2 db = { b.x - c.x, b.y - c.y };
    return da.x * db.y - da.y * db.x;
}

bool vecmath::bb2d(DirectX::XMFLOAT2 a1, DirectX::XMFLOAT2 a2,
    DirectX::XMFLOAT2 b1, DirectX::XMFLOAT2 b2)
{
    XMFLOAT2 atl = { fminf(a1.x, a2.x), fminf(a1.y, a2.y) };
    XMFLOAT2 abr = { fmaxf(a1.x, a2.x), fmaxf(a1.y, a2.y) };
    XMFLOAT2 btl = { fminf(b1.x, b2.x), fminf(b1.y, b2.y) };
    XMFLOAT2 bbr = { fmaxf(b1.x, b2.x), fmaxf(b1.y, b2.y) };
    bool xin =
        (atl.x <= btl.x && btl.x <= abr.x) ||
        (atl.x <= bbr.x && bbr.x <= abr.x) ||
        (btl.x <= atl.x && atl.x <= bbr.x);
    bool yin =                  
        (atl.y <= btl.y && btl.y <= abr.y) ||
        (atl.y <= bbr.y && bbr.y <= abr.y) ||
        (btl.y <= atl.y && atl.y <= bbr.y);

    return xin && yin;
}

bool vecmath::bb2d(DirectX::XMFLOAT2 atl, DirectX::XMFLOAT2 abr, DirectX::XMFLOAT2 b)
{
    bool xin = atl.x <= b.x && b.x <= abr.x;
    bool yin = atl.y <= b.y && b.y <= abr.y;
    return xin && yin;
}

int vecmath::segments2d(DirectX::XMFLOAT2 a1, DirectX::XMFLOAT2 a2, DirectX::XMFLOAT2 b1, DirectX::XMFLOAT2 b2)
{
    float ax = a1.x - a2.x;
    float bx = b1.x - b2.x;
    float dx = a1.x - b1.x;

    float ay = a1.y - a2.y;
    float by = b1.y - b2.y;
    float dy = a1.y - b1.y;

    float t = dx * by - dy * bx;
    float u = - (ax * dy - ay * dx);
    float v = ax * by - ay * bx;
    if (fabsf(v) < EPS * fminf(fminf(ax,bx),fminf(ay,by)))
    {
        return 0; // lines are parallel
        if (fabsf(u) < EPS && fabsf(t) < EPS && bb2d(a1, a2, b1, b2))
            return v < 0 ? -1 : 1;
        else return 0;
    }
    t /= v; u /= v;
    if (-EPS <= t && t <= 1+EPS && -EPS <= u && u <= 1+EPS)
        return v < 0 ? -1 : 1;
    else return 0;
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
