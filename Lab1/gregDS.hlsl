cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};
cbuffer cbDeBoor : register(b3)
{
    int isDeBoor;
    int detail;
};

struct DS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    //float3 vNormal : NORMAL;
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

void BezierBasis(out float b3[4], out float db3[4], float t)
{
    float b2[3], b1[2];
    
    float mt = 1 - t;
    b1[0] = mt;
    b1[1] = t;
	
    b2[0] = b1[0] * mt;
    b2[1] = b1[0] * t + b1[1] * mt;
    b2[2] = b1[1] * t;
	
    b3[0] = b2[0] * mt;
    b3[1] = b2[0] * t + b2[1] * mt;
    b3[2] = b2[1] * t + b2[2] * mt;
    b3[3] = b2[2] * t;
    
    db3[0] = -b2[0] * 3;
    db3[1] = (b2[0] - b2[1]) * 3;
    db3[2] = (b2[1] - b2[2]) * 3;
    db3[3] = b2[2] * 3;
}
#define NUM_CONTROL_POINTS 20


[domain("isoline")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    float3 p[16];
    uv.y *= 1 + 1.0 / detail;
    
    p[0] = patch[0].cpoint;
    p[1] = patch[1].cpoint;
    p[2] = patch[2].cpoint;
    p[3] = patch[5].cpoint;
    
    p[4] = patch[17].cpoint;
    if (uv.y + uv.x == 0 || uv.y + 1 - uv.x == 0)
        
    {
        p[5] = (patch[3].cpoint + patch[19].cpoint) / 2;
        p[6] = (patch[4].cpoint + patch[8].cpoint) / 2;

    }
    else
    {
        p[5] = (patch[3].cpoint * uv.y + patch[19].cpoint * (1 - uv.x))
        / (uv.y + 1 - uv.x);
        p[6] = (patch[4].cpoint * uv.y + patch[8].cpoint * uv.x)
        / (uv.y + uv.x);
    }
    p[7] = patch[6].cpoint;
    
    p[8] = patch[16].cpoint;
    
    if (1 - uv.y + uv.x == 0 || 1 - uv.y + 1 - uv.x == 0)
    {
        p[9] = (patch[14].cpoint + patch[18].cpoint) / 2;
        p[10] = (patch[13].cpoint + patch[9].cpoint) / 2;
    }
    else
    {
        p[9] = (patch[14].cpoint * (1 - uv.y) + patch[18].cpoint * (1 - uv.x))
        / (1 - uv.y + 1 - uv.x);
        p[10] = (patch[13].cpoint * (1 - uv.y) + patch[9].cpoint * uv.x)
        / (1 - uv.y + uv.x);
    }
    
    p[11] = patch[7].cpoint;
    
    p[12] = patch[15].cpoint;
    p[13] = patch[12].cpoint;
    p[14] = patch[11].cpoint;
    p[15] = patch[10].cpoint;
    
    DS_OUTPUT output;
    float bx[4], by[4];
    float dbx[4], dby[4];
    BezierBasis(bx, dbx, uv.x);
    BezierBasis(by, dby, uv.y);

    float3 pos;
    float3 dx, dy;
    pos = dx = dy = float3(0, 0, 0);
    
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
        {
            pos += bx[x] * by[y] * p[x + y * 4];
            dx += dbx[x] * by[y] * p[x + y * 4];
            dy += bx[x] * dby[y] * p[x + y * 4];
        }
    
    output.vPosition = float4(pos, 1);
    output.vPosition = mul(projMatrix, output.vPosition);
    
    //output.vNormal = mul(projMatrix, float4(normalize(cross(dx, dy)), 0)).xyz;
    //output.vNormal = normalize(output.vNormal);
	
    return output;
}
