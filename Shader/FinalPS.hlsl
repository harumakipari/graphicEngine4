#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Sampler.hlsli"

Texture2D colorTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D depthTexture : register(t3);
Texture2D bloomTexture : register(t4);
Texture2D fogTexture : register(t5);
Texture2D ssaoTexture : register(t6);
Texture2D ssrTexture : register(t7);
Texture2DArray cascadedShadowMaps : register(t9);

Texture3D noise3D : register(t20); // ノイズテクスチャ


cbuffer SSAO_CONSTANTS_BUFFER : register(b5)
{
    float radius;
    float bias;
    float power;
    float split_u;
}

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

float3 ApplyShadow(inout float3 color, in float4 positionWorldSpace, in float depthViewSpace, in float2 shadowMapDimensions, in float3 randSeed)
{
    float shadowFactor = 0.0;
	
    // カスケードされたビューフラスタムボリュームの層を見つける
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
	// 外側の遠方パネル
    if (cascadeIndex == -1)
    {
        return color;
    }
	
	// ワールド空間からライトビュークリップ空間へ、そしてNDCへ
    float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
    positionLightSpace /= positionLightSpace.w;
	// ndc からテクスチャ空間へ
    positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
    positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;

#if 1
	// 硬い影
    shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisionSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
#else
	// ソフトシャドウ
    const float2 sampleScale = (0.5 * shadow_filter_radius) / shadowMapDimensions;
    float accum = 0.0;
    for (uint sample_index = 0; sample_index < shadow_sample_count; ++sample_index)
    {
        float2 sampleOffset;
        float4 seed = float4(randSeed, sample_index);
        uint random = (uint) (64.0 * frac(sin(dot(seed, float4(12.9898, 78.233, 45.164, 94.673))) * 43758.5453)) % 64;
        sampleOffset = poisson_samples[random] * sampleScale;

        float2 sample_position = positionLightSpace.xy + sampleOffset;
        accum += cascadedShadowMaps.SampleCmpLevelZero(comparisionSamplerState, float3(sample_position, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
    }
    shadowFactor = accum / shadow_sample_count;
#endif
	
#if 1
    if (colorizeCascadedLayer)
    {
        const float3 colors[4] =
        {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
            { 1, 1, 0 },
        };
        return color * lerp(shadowColor, 1.0, shadowFactor) * colors[cascadeIndex];
    }
#endif
	
    return color * lerp(shadowColor, 1.0, shadowFactor);
}


// トーンマップ
float3 JodieReinhardToneMap(float3 c)
{
    float l = dot(c, float3(0.2126, 0.7152, 0.0722));
    float3 tc = c / (c + 1.0);

    return lerp(c / (l + 1.0), tc, tc);
}

// 
float3 CalculatedFogColor(float2 uv, float depth)
{
    uint2 depthMapDimensions;
    uint depthMipLevel = 0, numberOfSamples, levels;
    fogTexture.GetDimensions(depthMipLevel, depthMapDimensions.x, depthMapDimensions.y, numberOfSamples);
    
    float fogFacter = 0;
    if (enableBlur)
    {
        float accumulatedRadiance = 0.0;
        float accumulatedWeight = 0.0;
        const float radius = 4.0;
        for (float x = -radius; x <= radius; x += 1.0)
        {
            for (float y = -radius; y <= radius; y += 1.0)
            {
                float2 offset = float2(x, y) / depthMapDimensions;
                float2 texcoord = uv + offset;
                
                float sampledRadiance = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], texcoord).x;

                float distance = x * x + y * y;
                const float sigma = 2.0 * radius * radius;
                float domainGaussian = exp(-distance / sigma);

#if 1
                float sampledDepth = depthTexture.Sample(samplerStates[POINT], texcoord).x;
                distance = (depth - sampledDepth) * (depth - sampledDepth);
                const float sigma2 = 0.0001;
                float rangeGaussian = exp(-distance / sigma2);
#else
                float sampledDepthNdc = depthTexture.Sample(samplerStates[POINT], texcoord).x;
                // NDC → View に変換
                float4 ndc = float4(texcoord.x * 2 - 1, texcoord.y * -2 + 1, sampledDepthNdc, 1);
                float4 view = mul(ndc, inverseProjection);
                view /= view.w;
                const float sigma2 = 0.0001;
                float sampledLinearDepth = view.z;
                float diff = depth - sampledLinearDepth;
                float rangeGaussian = exp(-(diff * diff) / sigma2);
#endif
                accumulatedRadiance += sampledRadiance * domainGaussian * rangeGaussian;
                accumulatedWeight += domainGaussian * rangeGaussian;
            }
        }
        fogFacter = accumulatedRadiance / max(accumulatedWeight, 0.00001f);
    }
    else
    {
        fogFacter = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], uv).x;
    }

    float3 finalFogColor = fogColor.rgb * fogColor.a * max(0, fogFacter);
    return finalFogColor;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, numberOfLevel, levels;
    colorTexture.GetDimensions(mipLevel, width, height, numberOfLevel);

    uint2 depthMapDimensions;
    depthTexture.GetDimensions(mipLevel, depthMapDimensions.x, depthMapDimensions.y, numberOfLevel);

    uint2 shadowMapDimensions;
    cascadedShadowMaps.GetDimensions(mipLevel, shadowMapDimensions.x, shadowMapDimensions.y, numberOfLevel, levels);


    // シーンからライティング済みのカラーテクスチャ
    float4 color = colorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);

    // シーンから深度値を取得
    float depthNdc = depthTexture.Sample(samplerStates[POINT], pin.texcoord).x;

    float4 positionNdc;
    // uv -> ndc 
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depthNdc;
    positionNdc.w = 1;

    // ndc -> view 
    float4 positionViewSpace = mul(positionNdc, inverseProjection); // ndc → clip 
    positionViewSpace = positionViewSpace / positionViewSpace.w; // clip -> view 
    //return float4((positionViewSpace.z) / 100.0, 0, 0, 1);
    // ndc -> world 
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace /= positionWorldSpace.w;

    if (enableCascadedShadowMaps)
    {
        color.rgb = ApplyShadow(color.rgb, positionWorldSpace, (positionViewSpace.z), shadowMapDimensions, positionNdc.xyz);
    }

#if 0
    // フォグの処理
    if (enableFog)
    {
        float curr_depth = depthNdc;
        float4 sum = float4(0.0, 0.0, 0.0, 0.0);
        float4 sample;
        float radius = 4.0;
        float2 pos;
        float i, j;
        float sigma = 2.0 * radius * radius;
        float domainGaussian = 0.0;
        float weight = 0.0;
        float distance = 0.0;
        float sigma2 = 0.01;
        for (i = -radius; i <= radius; i += 1.0)
        {
            for (j = -radius; j <= radius; j += 1.0)
            {
                float dx = i / depthMapDimensions.x;
                float dy = j / depthMapDimensions.y;
                pos.x = pin.texcoord.x + dx;
                pos.y = pin.texcoord.y + dy;
                sample = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pos);

                distance = i * i + j * j;
                domainGaussian = exp(-distance / sigma);
			
                float sampleDepth = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pos).x;
                distance = (curr_depth - sampleDepth) * (curr_depth - sampleDepth);
                float rangeGaussian = exp(-distance / sigma2);
			
                sum += sample * domainGaussian * rangeGaussian;
                weight += domainGaussian * rangeGaussian;
            }
        }
        float4 volumetricLightColor = sum / weight;
	
	    //return volumetric_light_color;
	
        color.rgb = color.rgb /** volumetric_light_color.a */ + volumetricLightColor.rgb;

        //float3 fogColor = CalculatedFogColor(pin);
        //color.rgb += fogColor;
        //return float4(fogColor, 1);
    }

#else
    if (enableFog)
    {
        float linearDepth = positionViewSpace.z;
        //float3 fogColor = CalculatedFogColor(pin.texcoord, linearDepth);
        float3 fogColor = CalculatedFogColor(pin.texcoord, depthNdc);
        color.rgb += fogColor;
    }
#endif

    // ブルーム処理
    if (enableBloom)
    {
        float4 bloom = bloomTexture.Sample(samplerStates[POINT], pin.texcoord);
        color.rgb += bloom.rgb;
    }

    // SSRの処理
    if (enableSSR)
    {
        //float3 reflectColor = reflectionTexture.Sample(samplerStates[LINEAR_CLAMP], pin.texcoord).rgb;
        //return float4(reflectColor.rgb, 1);

        color.rgb += ssrTexture.Sample(samplerStates[LINEAR_CLAMP], pin.texcoord).rgb;
    }

    // SSAOの処理
    const float radius = 4.0;
    const float sigma = 2.0 * radius * radius;
    const float sigma2 = 0.01;
    float currDepth = depthNdc;
    float weight = 0.0;
	
    float accumulatedOcclusion = 0;
	
    for (float i = -radius; i <= radius; i += 1.0)
    {
        for (float j = -radius; j <= radius; j += 1.0)
        {
            float dx = i / width;
            float dy = j / height;
            float2 uv = float2(pin.texcoord.x + dx, pin.texcoord.y + dy);
			
            float distance = i * i + j * j;
            float domainGaussian = exp(-distance / sigma);
			
            float sampleDepth = depthTexture.SampleLevel(samplerStates[POINT], uv, 0).x;
            distance = (currDepth - sampleDepth) * (currDepth - sampleDepth);
            float rangeGaussian = exp(-distance / sigma2);
			
			//  サンプル遮蔽（環境）係数
            float sampleOcclusion = ssaoTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], uv, 0).x;
            accumulatedOcclusion += sampleOcclusion * domainGaussian * rangeGaussian;

            weight += domainGaussian * rangeGaussian;
        }
    }
    //if (pin.texcoord.x > split_u)
    {
        float occlusion = accumulatedOcclusion / weight;
        color *= occlusion;
    }

    // トーンマップ
    color.rgb = JodieReinhardToneMap(color.rgb);


    return color;
}