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

VSOut main(VSIn i)
{
	VSOut o;
	o.world = mul(worldMatrix, float4(i.pos.xyz, 1));
	o.pos = mul(projMatrix, mul(viewMatrix, o.world));
	o.uv = float2(0, 0);
	return o;
}