#include "Constants.hlsli"
#include "GltfModel.hlsli"
#include "FilterFunctions.hlsli"

#define BASECOLOR_TEXTURE 0 
#define METALLIC_ROUGHNESS_TEXTURE 1 
#define NORMAL_TEXTURE 2 
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4 
Texture2D<float4> materialTextures[5] : register(t1);


#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState samplerStates[5] : register(s0);

GBUFFER_PS_OUT main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace)
{
    GBUFFER_PS_OUT pout;
    const float GAMMA = 2.2;
    const MaterialConstants m = materials[material];

    float4 baseColorFactor = m.pbrMetallicRoughness.baseColorFactor;
    const int baseColorTexture = m.pbrMetallicRoughness.basecolorTexture.index;

    //float4 baseColor = baseColorFactor;

    if (baseColorTexture > -1)
    {
        float4 sampled = materialTextures[BASECOLOR_TEXTURE].Sample(samplerStates[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        baseColorFactor *= sampled;

    }
    
    if (m.alphaMode == 0 /*OPAQUE*/)
    {
        baseColorFactor.a = 1.0;
    }
    //if (m.alphaMode == 1 && baseColorFactor.a < 1.0)
    //{
    //    pout.gbuffer3Color = float4(1, 0, 0, 1);
    //    return pout;
    //    discard;
    //}
    if (baseColorFactor.a < m.alphaCutoff)
    {
        //pout.gbuffer3Color = float4(1, 0, 0, 1);
        discard;
    }
    
    float3 emissiveFactor = m.emissiveFactor;
    
    const int emissiveTexture = m.emissiveTexture.index;
    if (emissiveTexture > -1)
    {
        float4 sampled = materialTextures[EMISSIVE_TEXTURE].Sample(samplerStates[2], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emissiveFactor *= sampled.rgb;
    }
    
    float roughnessFactor = m.pbrMetallicRoughness.roughnessFactor;
    float metallicFactor = m.pbrMetallicRoughness.metallicFactor;
    const int metallicRoughnessTexture = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (metallicRoughnessTexture > -1)
    {
        float4 sampled = materialTextures[METALLIC_ROUGHNESS_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        roughnessFactor *= sampled.g;
        metallicFactor *= sampled.b;
    }
    
    float occlusionFactor = 1.0;
    const int occlusionTexture = m.occlusionTexture.index;
    if (occlusionTexture > -1)
    {
        float4 sampled = materialTextures[OCCLUSION_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        occlusionFactor *= sampled.r;
    }
    const float occlusionStrength = m.occlusionTexture.strength;

    const float3 f0 = lerp(0.04, baseColorFactor.rgb, metallicFactor);
    const float3 f90 = 1.0;
    const float alphaRoughness = roughnessFactor * roughnessFactor;
    const float3 cDiff = lerp(baseColorFactor.rgb, 0.0, metallicFactor);
    
    const float3 P = pin.wPosition.xyz;
    const float3 V = normalize(cameraPositon.xyz - pin.wPosition.xyz);
    
    float3 N = normalize(pin.wNormal.xyz);
    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    //For a back-facing surface, the tangential basis vectors are negated.
    if (isFrontFace == false)
    {
        T = -T;
        B = -B;
        N = -N;
    }
    
    const int normalTexture = m.normalTexture.index;
    if (normalTexture > -1)
    {
        float4 sampled = materialTextures[NORMAL_TEXTURE].Sample(samplerStates[LINEAR], pin.texcoord);
        float3 normalFactor = sampled.xyz;
        normalFactor = (normalFactor * 2.0) - 1.0;
        normalFactor = normalize(normalFactor * float3(m.normalTexture.scale, m.normalTexture.scale, 1.0));
        N = normalize((normalFactor.x * T) + (normalFactor.y * B) + (normalFactor.z * N));
    }

    float4 color = baseColorFactor;
    {// カラー調整
      // RGB > HSV に変換
        color.rgb = RGB2HSV(color.rgb);

    // 色相調整
        color.r += modelHueShift;

    // 彩度調整
        color.g *= modelSaturation;

    // 明度調整
        color.b *= modelBrightness;

    // HSV > RGB に変換
        color.rgb = HSV2RGB(color.rgb);
    }
    pout.albedo = color;
    //pout.position = mul(pin.wPosition, view); // to viewSpace
    //pout.gbuffer1Normal = mul(float4(N.xyz, 0), view); //to viewSpace;
    pout.position = pin.wPosition; // to viewSpace
    pout.gBuffer3Normal = float4(N.xyz, 0); //to viewSpace;
    pout.emissive = float4(emissiveFactor, 0); // 元々wは１だったがスカイマップなどの時に使用するため０に変更
    pout.material = float4(metallicFactor, roughnessFactor, occlusionFactor, occlusionStrength);
    
    return pout;
}