#include"structs.hlsli"

Texture2D<float4> trim : register(t0);
cbuffer cbColor : register(b0)
{
	float4 c1;
	float4 c2;
	float4 c3;
	float4 c4;
};
cbuffer cbTint : register(b1)
{
    float4 tint;
};
cbuffer cbMode : register(b3)
{
    int3 _123;
    int trimming;
    int4 _5678;
};

float4 main(VSOut o) : SV_TARGET
{
	if (o.uv.y > 10)
	{
		if (o.uv.x < 0.5)
			return tint*c1;
		if (o.uv.x < 1.5)
			return tint*c2;
		return tint*c3;
	}
	float d = o.uv.x * o.uv.x + o.uv.y * o.uv.y;
	if (d > 1)
		discard;
	
    int width, height, x;
    trim.GetDimensions(0, width, height, x);
    return tint * (c1 * (1 - trimming) + trimming * trim.Load(
        int3(o.uv * int2(height, width), 0),
        int2(0, 0)));
}