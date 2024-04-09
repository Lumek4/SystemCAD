struct VSIn {
	float3 pos : POSITION;
};
struct VSOut {
	float4 pos : SV_POSITION;
	float4 world : POSITION1;
	float2 uv : TEXCOORD;
};
struct GSOut
{
    float4 pos : SV_POSITION;
};