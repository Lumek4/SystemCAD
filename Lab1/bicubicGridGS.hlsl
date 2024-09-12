#include"structs.hlsli"
cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

struct GSIn
{
    float3 pos : CPOINT;
};


float4 tform(float3 input)
{
    float4 p = mul(projMatrix, mul(viewMatrix, float4(input, 1)));
    return p / p.w;
}

[maxvertexcount(64)]
void main(
	InputPatch<GSIn, 16> input,
	inout LineStream<GSOut> output
)
{
    GSOut element;
    for (uint i = 0; i < 4; i++)
    {
        for (uint k = 0; k < 4; k++)
        {
            element.pos = tform(input[k + i * 4].pos);
            element.uv = float2(k/3.0f, i/3.0f);
            output.Append(element);
        }
        output.RestartStrip();
        for (uint l = 0; l < 4; l++)
        {
            element.pos = tform(input[i + l * 4].pos);
            element.uv = float2(i / 3.0f, l / 3.0f);
            output.Append(element);
        }
        output.RestartStrip();
    }
}