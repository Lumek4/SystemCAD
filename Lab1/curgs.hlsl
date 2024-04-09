#include"structs.hlsli"

float ratio;
float size;
cbuffer cbView : register(b1)
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
	matrix projMatrix;
};

[maxvertexcount(6)]
void main(
	point VSOut input[1],
	inout LineStream< VSOut > output
)
{
	float4 off[3];
	off[0] = float4(1,0,0,0);
	off[1] = float4(0,1,0,0);
	off[2] = float4(0,0,1,0);
	float4 pos = input[0].pos;

	for (int i = 0; i < 3; i++)
	{
		VSOut o;
		o.pos = pos;
		o.uv = float2(i, 11);
		o.world = input[0].world;
		output.Append(o);

		o.pos = mul(projMatrix, mul(viewMatrix, input[0].world + off[i]* size*o.pos.w));
		//o.pos.xyzw /= o.pos.w;
		o.uv = float2(i, 11);
		o.world = input[0].world + off[i]*pos.w;
		output.Append(o);
		output.RestartStrip();
	}
}