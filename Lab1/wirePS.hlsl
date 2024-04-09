#include"structs.hlsli"

cbuffer cbColor : register(b0)
{
    float4 c1;
};


float4 main(GSOut i) : SV_TARGET
{
	return c1;
}