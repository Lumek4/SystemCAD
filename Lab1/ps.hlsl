#include"structs.hlsli"

cbuffer cbColor : register(b0)
{
	float4 c1;
	float4 c2;
	float4 c3;
	float4 c4;
};

float4 main(VSOut o) : SV_TARGET
{
	if (o.uv.y > 10)
	{
		if (o.uv.x < 0.5)
			return c1;
		if (o.uv.x < 1.5)
			return c2;
		return c3;
	}
	float d = o.uv.x * o.uv.x + o.uv.y * o.uv.y;
	if (d > 1)
		discard;
	return c1;
}