#include"structs.hlsli"

float ratio;
float size;

[maxvertexcount(4)]
void main(
	point VSOut input[1],
	inout TriangleStream< VSOut > output
)
{
	float2 off[4];
	off[0] = float2(+0.5, +0.5);
	off[1] = float2(+0.5, -0.5);
	off[2] = float2(-0.5, +0.5);
	off[3] = float2(-0.5, -0.5);
	float4 pos = input[0].pos;

	for (int i = 0; i < 4; i++)
	{
		VSOut o;
		o.pos = float4(pos.xy + off[i].xy* size *float2(1,ratio)* pos.w, 0, pos.w);
		o.uv = off[i]*2;
		o.world = input[0].world;
		output.Append(o);
	}
}