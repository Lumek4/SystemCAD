cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};
cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};
cbuffer cbDeBoor : register(b3)
{
    int isDeBoor;
    int detail;
    int flip;
};

struct VS_OUTPUT
{
    float3 cpoint : CPOINT;
};

struct HS_OUTPUT
{
    float3 cpoint : CPOINT;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[2] : SV_TessFactor;
    //float EdgeTessFactor[4] : SV_TessFactor;
    //float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 20

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT Output;

  //  Output.EdgeTessFactor[0] =
		//Output.EdgeTessFactor[1] =
		//Output.EdgeTessFactor[2] =
		//Output.EdgeTessFactor[3] =
		//Output.InsideTessFactor[0] =
		//Output.InsideTessFactor[1] = detail;
    
    Output.EdgeTessFactor[0] = detail + 1;
    Output.EdgeTessFactor[1] = detail;

    return Output;
}

[domain("isoline")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[outputcontrolpoints(20)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_OUTPUT main(
	InputPatch<VS_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_OUTPUT output;
    if (flip != 0)
        output.cpoint = mul(viewMatrix, float4(ip[(i + 5)%20].cpoint, 1)).xyz;
    else
        output.cpoint = mul(viewMatrix, float4(ip[i].cpoint, 1)).xyz;

    return output;
}
