cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

struct HS_CONSTANT_OUTPUT
{
    float edges[2] : SV_TessFactor;
};
struct HS_OUTPUT
{
    float3 cpoint : CPOINT;
};
struct DS_OUTPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float3 Bezier1(float3 c0[4], float t)
{
    float1 mt = 1 - t;
    return c0[0] * mt + c0[3] * t;
}
float3 Bezier2(float3 c0[4], float t)
{
    float3 c1[2];
    
    float1 mt = 1 - t;
    c1[0] = c0[0] * mt + c0[1] * t;
    c1[1] = c0[1] * mt + c0[3] * t;
    
    return c1[0] * mt + c1[1] * t;
}
float3 Bezier3(float3 c0[4], float t)
{
    float3 c1[3], c2[2];
    
    float1 mt = 1 - t;
    c1[0] = c0[0] * mt + c0[1] * t;
    c1[1] = c0[1] * mt + c0[2] * t;
    c1[2] = c0[2] * mt + c0[3] * t;
    
    c2[0] = c1[0] * mt + c1[1] * t;
    c2[1] = c1[1] * mt + c1[2] * t;
    
    return c2[0] * mt + c2[1] * t;
}

[domain("isoline")]
DS_OUTPUT main(
    HS_CONSTANT_OUTPUT input,
    OutputPatch<HS_OUTPUT, 4> op,
    float2 uv : SV_DomainLocation)
{
    DS_OUTPUT output;
    float t = uv.x;
    float3 c[4] = { op[0].cpoint, op[1].cpoint, op[2].cpoint, op[3].cpoint };
    //c[1] = float3(0, 0, 0);
    float3 pos = any(isnan(c[0])) ? float3(c[0]) :
                 any(isnan(c[1])) ? Bezier1(c, t) :
                 any(isnan(c[2])) ? Bezier2(c, t) :
                    Bezier3(c, t);
    output.position = float4(pos, 1.0f);
    output.position = mul(projMatrix, output.position);
    output.uv = uv;
    return output;
}