#include "Sampler.hlsli"
#include "Constants.hlsli"
#include "Common.hlsli"


Texture2D depthTexture : register(t0);
Texture2D<float4> sceneNormalTexture : register(t1);
StructuredBuffer<float3> kernelPoints : register(t2);
StructuredBuffer<float3> noise : register(t3);

#define ALCHEMY_AO

cbuffer SSAO_CONSTANTS_BUFFER : register(b5)
{
    float radius;
    float bias;
    float power;
    float pad;
}


float4 main(float4 svPosition : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    float3 normal = sceneNormalTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], texcoord, 0).xyz; // world‹óŠÔ
    normal = mul(float4(normal, 0), view).xyz; //@world‹óŠÔ -> view‹óŠÔ

    float depth = depthTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], texcoord, 0);
#if 1
    if (depth > 0.9999999)
    {
        discard;
    }
#endif	
    float4 ndc;
    ndc.xy = UvToNdc(texcoord);
    ndc.z = depth;
    ndc.w = 1.0;
	
	// ndc -> view‹óŠÔ@
    float4 position = mul(ndc, inverseProjection);
    position /= position.w;
	
	// TBN‚ÍÚü‹óŠÔ‚©‚ç‹‹óŠÔ‚Ö‚Ì•ÏŠ·s—ñ‚Å‚ ‚é
    float3 randomVec = noise[(svPosition.x % 4) + 4 * (svPosition.y % 4)]; // Random kernel rotation
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
	
    const int kernel_size = 64;
	
    float occlusion = 0.0; // accumulated value
    for (int kernel = 0; kernel < kernel_size; ++kernel)
    {
        float3 sample_position = mul(kernelPoints[kernel], TBN); // from tangent to view-space
        sample_position = position.xyz + sample_position * radius;
		
		// Find a view-space scene intersection point on the ray.
        float4 intersection = mul(float4(sample_position, 1.0), projection); // from view to clip-space
        intersection /= intersection.w; // from clip-space to ndc
        intersection.z = depthTexture.SampleLevel(samplerStates[LINEAR_BORDER_WHITE], NdcToUv(intersection).xy, 0);
        intersection = mul(intersection, inverseProjection); // from ndc to view-space
        intersection /= intersection.w; // perspective divide
		
		// Alchemy AO
        float3 v = intersection.xyz - position.xyz;
        const float beta =bias; // bias distance
        const float epsilon = 0.001; //@ƒ[ƒœZ‚ğ–h~‚·‚é‚½‚ß‚Ì¬‚³‚È’l
        occlusion += max(0, dot(normal, v) - position.z * beta) / (dot(v, v) + epsilon);
    }
	
    const float sigma = 0.3;
    occlusion = max(0.0, 1.0 - (2.0 * sigma * occlusion / kernel_size));
	
    return power > 0.0 ? pow(occlusion, power) : 1.0; // TODO
}