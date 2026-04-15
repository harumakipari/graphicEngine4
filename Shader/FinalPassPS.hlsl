#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Sampler.hlsli"
#include "FilterFunctions.hlsli"
#include "Lights.hlsli"
#include "ModelType.hlsli"
#include "ShaderFunctions.hlsli"

Texture2D colorTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D depthTexture : register(t3);
Texture2D bloomTexture : register(t4);
Texture2DArray cascadedShadowMaps : register(t5);

float2 NdcToUv(float2 ndc)
{
    float2 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    return uv;
}


// ToneMap
float3 JodieReinhardTonemap(float3 c)
{
    float l = dot(c, float3(0.2126, 0.7152, 0.0722));
    float3 tc = c / (c + 1.0);

    return lerp(c / (l + 1.0), tc, tc);
}


float4 CalculatedPositionNDC(VS_OUT pin)
{
    float depthNdc = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    return positionNdc;
}

float CalculatedCascadedShadowFactor(VS_OUT pin)
{
    // CASCADED_SHADOW_MAPS
    //float4 sampledColor = colorTexture.Sample(linearBorderBlackSamplerState, pin.texcoord);
    //float3 sampleColor = sampledColor.rgb;
    //float sampleAlpha = sampledColor.a;

    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;

    // ndc to world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;

    // Apply cascaded shadow mapping
    // Find alayer of cascaded view frustum volume
    float depthViewSpace = positionViewSpace.z;
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depthViewSpace)
        {
            cascadeIndex = layer;
            break;
        }
    }
    float shadowFactor = 1.0;
    if (cascadeIndex > -1)
    {
        // world space to loght view clip space, and to ndc
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w;
        // ndc to texture space
        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisionSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
        //shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
        
        return shadowFactor;
        float3 layerColor = 1;
#if 1
        if (colorizeCascadedLayer)
        {
            const float3 layerColors[4] =
            {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
                { 1, 1, 0 },
            };
            layerColor = layerColors[cascadeIndex];
        }
#endif
        //color *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
        //color *= float4(lerp(shadowColor, 1.0, shadowFactor) * layerColor, color.a);
        
        //sampleColor *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
    }
    return shadowFactor;
    //return sampleColor;
}


float rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);

}

float3 AdjustSaturation(float3 color, float saturation)
{
    // ‹P“xiRec.709j
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    return lerp(luma.xxx, color, saturation);
}


float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, number_of_level;
    colorTexture.GetDimensions(mipLevel, width, height, number_of_level);

    float4 color = colorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    float alpha = color.a;
    float depthNdc = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;

    // GBuffer‚ð‘‚«ž‚ñ‚Å‚¢‚È‚¢—Ìˆæ‚ÍColor‚»‚Ì‚Ü‚Ü•Ô‚·
     // ‚±‚êUI‚Ì‚½‚ß‚É
    bool isSky = (depthNdc == 0.0 || depthNdc >= 1.0);
    if (isSky)
    {
        return float4(color.rgb, 1.0);
    }

    

    float4 positionNdc;
    // texture space to ndc
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    
    // ndc to view space
    float4 positionViewSpace = mul(positionNdc, inverseProjection);
    positionViewSpace = positionViewSpace / positionViewSpace.w;
    
    // ndc to world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
    
    uint2 dimensions;
    uint numberOfLevels;
    positionTexture.GetDimensions(mipLevel, dimensions.x, dimensions.y, numberOfLevels);
    

    uint mip_level = 0, number_of_samples;
    uint2 depthDimensions;
    depthTexture.GetDimensions(mip_level, depthDimensions.x, depthDimensions.y, number_of_samples);
    float depth = depthTexture.Sample(samplerStates[LINEAR_CLAMP], pin.texcoord).x;

    float shadowFactor = CalculatedCascadedShadowFactor(pin);
    
    // Apply cascaded shadow mapping
    // Find alayer of cascaded view frustum volume
    float depthViewSpace = positionViewSpace.z;
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depthViewSpace)
        {
            cascadeIndex = layer;
            break;
        }
    }
    if (cascadeIndex > -1)
    {
        float3 layerColor = 1;
#if 1
        if (colorizeCascadedLayer)
        {
            const float3 layerColors[4] =
            {
                { 1, 0, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
                { 1, 1, 0 },
            };
            layerColor = layerColors[cascadeIndex];
        }
#endif
        if (enableCascadedShadowMaps)
        {
            color.rgb *= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
        }
    }
    
    
    
#if 1
    //BLOOM
    if (enableBloom)
    {
        float4 bloom = bloomTexture.Sample(samplerStates[POINT], pin.texcoord);
        color.rgb += bloom.rgb;
    }
    
    color.rgb = JodieReinhardTonemap(color.rgb);
    const float GAMMA = 2.2;
    color.rgb = pow(color.rgb, 1.0 / GAMMA);
    return float4(color.rgb, alpha);
#endif
}
