cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};
cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};


struct VS_OUTPUT
{
    float3 cpoint : CPOINT;
};
struct HS_CONSTANT_OUTPUT
{
    float edges[2] : SV_TessFactor;
};
struct HS_OUTPUT
{
    float3 cpoint : CPOINT;
};
HS_CONSTANT_OUTPUT HSConst(
    InputPatch<VS_OUTPUT, 4> ip,
    uint id : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT output;
    output.edges[0] = 1.0f; // Detail factor (see below for explanation)
    
    float4 cps[4];
    int ii = 0;
    for (int i = 0; i < 4; i++)
    {
        if (any(isnan(ip[i].cpoint)))
            continue;
        cps[ii] = mul(viewMatrix, float4(ip[i].cpoint, 1.0f));
        cps[ii] = mul(projMatrix, cps[ii]);
        cps[ii] /= cps[ii].w;
        ii++;
    }
    float l = 0;
    for (i = 0; i < ii; i++)
    {
        float4 p = cps[i] - cps[i + 1];
        float len = length(p);
        //if (!isnan(len))
            l = max(l, length(p));

    }
    output.edges[1] = l*1024.0f; // Density factor
    return output;
}
[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HS_OUTPUT main(
    InputPatch<VS_OUTPUT, 4> ip,
    uint id : SV_OutputControlPointID)
{
    HS_OUTPUT output;
    output.cpoint = mul(viewMatrix, float4(ip[id].cpoint, 1)).xyz;
    return output;
}