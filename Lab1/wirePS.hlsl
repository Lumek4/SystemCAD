#include"structs.hlsli"

Texture2D<float4> trim : register(t0);

cbuffer cbColor : register(b0)
{
    float4 c1;
};
cbuffer cbMode : register(b3)
{
    int3 _123;
    int trimming;
    int2 pos;
    int2 max;
};


float4 main(GSOut i) : SV_TARGET
{
    int width, height, x;
    trim.GetDimensions(0, width,height, x);
    return trimming*trim.Load(
        int3(((i.uv + pos) * int2(height, width)) / max, 0),
        int2(0, 0)) + (1 - trimming)*c1;
}