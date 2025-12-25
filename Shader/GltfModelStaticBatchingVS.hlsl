#include "GltfModel.hlsli"

VS_OUT main(float4 position : POSITION, float4 normal : NORMAL, float4 tangent : TANGENT, float2 texcoord : TEXCOORD)
{
    VS_OUT vout;

    position.w = 1;
    vout.position = mul(position, mul(world, viewProjection));
    vout.wPosition = mul(position, world);

    normal.w = 0;
    vout.wNormal = normalize(mul(normal, world));
    //vout.wNormal.xyz = normalize(mul(normal, inverseTransposeWorld).xyz);
    vout.wNormal.w = 0;

    float sigma = tangent.w;
    tangent.w = 0;
    vout.wTangent = normalize(mul(tangent, world));
    //vout.wTangent.xyz = normalize(mul(tangent, inverseTransposeWorld).xyz);
    vout.wTangent.w = sigma;

    vout.texcoord = texcoord;

    return vout;
}
