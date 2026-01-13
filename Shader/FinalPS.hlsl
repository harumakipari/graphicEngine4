#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Sampler.hlsli"

Texture2D colorTexture : register(t0);

Texture2D depthTexture : register(t3);


Texture2DArray cascadedShadowMaps : register(t8);

// texcoord -> ndc 空間に変換
float4 CalculatedPositionNDC(VS_OUT pin)
{
    float depthNdc = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    float4 positionNdc;
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;
    return positionNdc;
}


float CalculatedCascadedShadowFactor(VS_OUT pin)
{
    // uv -> ndc 
    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc -> view 
    float4 positionViewSpace = mul(positionNdc, inverseProjection); // ndc → clip 
    positionViewSpace = positionViewSpace / positionViewSpace.w; // clip -> view 

    // ndc -> world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;

    // カスケードシャドウマッピングを適用する
    // カスケードビューフラスタムボリュームのレイヤーを見つける
    float depthViewSpace = positionViewSpace.z; // view 空間の z はカメラからの距離
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


float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, number_of_level;
    colorTexture.GetDimensions(mipLevel, width, height, number_of_level);

    // シーンからライティング済みのカラーテクスチャ
    float4 color = colorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);

    //return float4(color);

    // uv -> ndc 
    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc -> view 
    float4 positionViewSpace = mul(positionNdc, inverseProjection); // ndc → clip 
    positionViewSpace = positionViewSpace / positionViewSpace.w; // clip -> view 

    float shadowFactor = CalculatedCascadedShadowFactor(pin);
    
    // カスケードシャドウマッピングを適用する
    // カスケードビューフラスタムボリュームのレイヤーを見つける
    float depthViewSpace = positionViewSpace.z; // view 空間の z はカメラからの距離
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
            float3 shadow= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
            //return float4(shadow, 1);
        }
    }

    return color;
}