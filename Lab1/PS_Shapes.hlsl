#include"structs.hlsli"

Texture2D<float4> trim : register(t0);

cbuffer cbColor : register(b0)
{
    float4 c1;
};
cbuffer cbTint : register(b1)
{
    float4 tint;
};
cbuffer cbMode : register(b3)
{
    int3 _123;
    int trimming;
    int2 pos;
    int2 max;
    int areas;
};


float4 main(GSOut i) : SV_TARGET
{
    float4 colors[] = {
		{1, 0, 0.25f, 1},
		{0.25f, 1, 0, 1},
		{0, 0.25f, 1, 1},
		{0.75f, 0.75f, 0, 1},
		{0, 0.75f, 0.75f, 1},
		{0.75f, 0, 0.75f, 1},
		{1, 0.25f, 0, 1},
		{0, 1, 0.25f, 1},
		{0.25f, 0, 1, 1},
		{0.25f, 0.25f, 0.75f, 1},
		{0.75f, 0.25f, 0.25f, 1},
		{0.25f, 0.75f, 0.25f, 1},
	};
    int width, height, x;
    trim.GetDimensions(0, width,height, x);
    float4 color = trim.Load(
        int3(((i.uv + pos.xy) * int2(height, width)) / max.xy, 0),
        int2(0, 0));
    int a = areas;
    for (int j = 0; j < 12; j++)
    {
        if (a & 1)
            if (dot(color - colors[j], color - colors[j]) < 0.001f)
                discard;
        a >>= 1;
    }
    return tint * (
    float4(1 - (i.uv.xy + pos.xy) / max.xy, 1, 1) * (1 - c1.a) + c1 * c1.a);
}