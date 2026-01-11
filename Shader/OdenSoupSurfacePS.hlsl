#include "GltfModel.hlsli"

Texture2D<float3> normalTexture : register(t12); // 水面法線マップ

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[5] : register(s0);

float4 main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET0
{
    float3 normalColor = normalTexture.Sample(samplerStates[ANISOTROPIC], pin.texcoord);
	return float4(normalColor, 1.0f);
}