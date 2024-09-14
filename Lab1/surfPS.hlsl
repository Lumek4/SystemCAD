cbuffer cbColor : register(b0)
{
    float4 c1;
    float4 c2;
    float4 c3;
    float4 c4;
};
cbuffer cbTint : register(b1)
{
    float4 tint;
};

struct DS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
};

float4 main(DS_OUTPUT i) : SV_TARGET
{
    float facingYou = abs(dot(i.vNormal, float3(0, 0, -1)));
    float backLight = 0;//    pow(1 - abs(2 * facingYou - 1), 10) * 0.5f;
    return tint*float4(clamp(facingYou + backLight, 0.5f, 1) * c1.rgb, 1);

}