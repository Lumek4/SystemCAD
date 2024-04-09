cbuffer cbWorld : register(b0)
{
    matrix worldMatrix;
};

cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

float3 main( float3 pos : POSITION ) : CPOINT
{
    return pos.xyz;
}