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

struct DS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct HS_OUTPUT
{
    float3 cpoint : CPOINT;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[2]			: SV_TessFactor;
	//float EdgeTessFactor[4]			: SV_TessFactor;
	//float InsideTessFactor[2]			: SV_InsideTessFactor;
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
    
    db3[0] = - b2[0] * 3;
    db3[1] = (b2[0] - b2[1]) * 3;
    db3[2] = (b2[1] - b2[2]) * 3;
    db3[3] = b2[2] * 3;
}
#define NUM_CONTROL_POINTS 16


[domain("isoline")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    float3 p[16];
    for (int i = 0; i < 16; i++)
        p[i] = patch[i].cpoint;
    uv.y *= 1 + 1.0 / detail;
    
    if(isDeBoor)
    {
        float3 extBezier[16];
        for (int y = 0; y < 4; y++)
        {
            extBezier[0 + y * 4] = lerp(p[0 + y * 4], p[1 + y * 4], 2.0 / 3);
            extBezier[1 + y * 4] = lerp(p[1 + y * 4], p[2 + y * 4], 1.0 / 3);
            extBezier[2 + y * 4] = lerp(p[1 + y * 4], p[2 + y * 4], 2.0 / 3);
            extBezier[3 + y * 4] = lerp(p[2 + y * 4], p[3 + y * 4], 1.0 / 3);
        }
        for (int x = 0; x < 4; x++)
        {
            p[x + 0 * 4] = lerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 2.0 / 3);
            p[x + 1 * 4] = lerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 1.0 / 3);
            p[x + 2 * 4] = lerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 2.0 / 3);
            p[x + 3 * 4] = lerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 1.0 / 3);
        }

        for (int y = 0; y < 4; y++)
        {
            extBezier[0 + y * 4] = lerp(p[0 + y * 4], p[1 + y * 4], 0.5);
            extBezier[1 + y * 4] = p[1 + y * 4];
            extBezier[2 + y * 4] = p[2 + y * 4];
            extBezier[3 + y * 4] = lerp(p[2 + y * 4], p[3 + y * 4], 0.5);
        }
        for (int x = 0; x < 4; x++)
        {
            p[x + 0 * 4] = lerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 0.5);
            p[x + 1 * 4] = extBezier[x + 1 * 4];
            p[x + 2 * 4] = extBezier[x + 2 * 4];
            p[x + 3 * 4] = lerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 0.5);
        }
    }
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
    
    float3 normal = normalize(cross(dx, dy));
    if (flip)
        normal = -normal;
    output.vPosition = float4(pos, 1);
    output.vPosition = mul(projMatrix, output.vPosition);
    output.uv = flip ? uv.yx : uv.xy;
	
    
	return output;
}
