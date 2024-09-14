#include"structs.hlsli"
cbuffer cbWorld : register(b0)
{
    matrix worldMatrix;
};

cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};
struct VSInUV
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};
GSOut main(VSInUV i)
{
    GSOut o;
    float4 world = mul(worldMatrix, float4(i.pos.xyz, 1));
    o.pos = mul(projMatrix, mul(viewMatrix, world));
    o.uv = i.uv;
    return o;
}