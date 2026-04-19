#include "ComputeParticle.hlsli"

Texture2D colorMap : register(t0);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[3] : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
    float4 col = colorMap.Sample(samplerStates[ANISOTROPIC], pin.texcoord) * pin.color;
    return col; // チーム制作用　(T_T)
    return float4(col.rgb * pin.emissive, 1); // 自作エンジンではこっちを使う
}