#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Sampler.hlsli"
#include "Lights.hlsli"

Texture2D colorTexture : register(t0);

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

// CSM 用の影係数を計算する関数
float CalculatedCascadedShadowFactor(VS_OUT pin, out int cascadeIndex)
{
    // uv -> ndc 
    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc -> view 
    float4 positionViewSpace = mul(positionNdc, inverseProjection); // ndc → clip 
    positionViewSpace = positionViewSpace / positionViewSpace.w; // clip -> view 

    // ndc -> world space
    float4 positionWorldSpace = mul(positionNdc, inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;

    // カスケードビューフラスタムボリュームのレイヤーを見つける
    float depthViewSpace = positionViewSpace.z; // view 空間の z はカメラからの距離
    // カメラからの距離からどのカスケードを使用するか選択する
    cascadeIndex = -1;
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
        //　world 空間 -> Light Clip 空間 (各カスケードに対応する Light View Projection 空間)
        float4 positionLightSpace = mul(positionWorldSpace, cascadedMatrices[cascadeIndex]);
        positionLightSpace /= positionLightSpace.w; // Light Clip 空間 -> ndc 空間
        // ndc 空間 -> texture 空間
        positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
        positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
        // シャドウマップの深度と現在ピクセルの深度を比較
        shadowFactor = cascadedShadowMaps.SampleCmpLevelZero(comparisionSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
        return shadowFactor; // 影の中
    }
    return shadowFactor; // 光が当たっている
}





// フォグ結果を「深度付きバイラテラルブラー」する
float3 CalculatedFogColor(VS_OUT pin)
{
    uint2 depthMapDimensions;
    uint depthMipLevel = 0, numberOfSamples, levels;
    fogTexture.GetDimensions(depthMipLevel, depthMapDimensions.x, depthMapDimensions.y, numberOfSamples);
    //depthTexture.GetDimensions(depthMipLevel, depthMapDimensions.x, depthMapDimensions.y, numberOfSamples);
    
    float fogFacter = 0;
    if (enableBlur)
    {
        // 深度取得
        float depth = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    
        float accumulatedRadiance = 0.0;
        float accumulatedWeight = 0.0;
        const float radius = 4.0;
        // 周囲のピクセルを確認　9*9 
        for (float x = -radius; x <= radius; x += 1.0)
        {
            for (float y = -radius; y <= radius; y += 1.0)
            {
                float2 offset = float2(x, y) / depthMapDimensions;
                float2 texcoord = pin.texcoord + offset;
                
                float sampledRadiance = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], texcoord).x;

                // 距離による重みでぼかす（近いピクセルほど影響大、遠いほど影響小）
                float distance = x * x + y * y;
                const float sigma = 2.0 * radius * radius;
                float domainGaussian = exp(-distance / sigma);

                // 深度による重みでぼかす (深度が近い → 重み大、深度が違う → 重みほぼ0）
                float sampledDepth = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], texcoord).x;
                distance = (depth - sampledDepth) * (depth - sampledDepth);
                const float sigma2 = 0.0001;
                float rangeGaussian = exp(-distance / sigma2);

                // 重み付き加算
                accumulatedRadiance += sampledRadiance * domainGaussian * rangeGaussian;
                accumulatedWeight += domainGaussian * rangeGaussian;
            }
        }
        // 正規化されたバイラテラルブラー結果
        fogFacter = accumulatedRadiance / max(accumulatedWeight, 0.00001f);
    }
    else
    {
        fogFacter = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
    }

    // fogColor.rgb → フォグの色  fogColor.a → 全体強度  fogFacter→ レイマーチ結果（ブラー済）
    float3 finalFogColor = fogColor.rgb * fogColor.a * max(0, fogFacter);
    return finalFogColor;
}



float calculate_shadows_and_lighting(float depth_view_space, float3 position_world_space)
{
    int cascade_index = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depth_view_space)
        {
            cascade_index = layer;
            break;
        }
    }
    if (cascade_index == -1)
    {
        return 1;
    }
    float4 position_light_space = mul(float4(position_world_space, 1.0), cascadedMatrices[cascade_index]);
    position_light_space /= position_light_space.w;

	// To texture space
    position_light_space.x = position_light_space.x * +0.5 + 0.5;
    position_light_space.y = position_light_space.y * -0.5 + 0.5;

    return cascadedShadowMaps.SampleCmpLevelZero(comparisionSamplerState, float3(position_light_space.xy, cascade_index), position_light_space.z - shadowDepthBias).x;

    //return shadow_map.SampleCmpLevelZero(comparison_sampler_state, float3(position_light_space.xy, cascade_index), position_light_space.z - effect_data.shadow_depth_bias).x;
}
#define PI 3.14159265358979
float compute_mie_scattering(float LoV)
{
    const float g = mieScatteringCoef;
    float mie_scattering = 1.0 - g * g;
    mie_scattering /= (4.0 * PI * pow(1.0 + g * g - (2.0 * g) * LoV, 1.5));
    return mie_scattering;
}

float3 compute_layleigh_scattering(float LoV)
{
    return 3 / (16.0 * PI) * (1.0 + LoV * LoV) * float3(5.8, 13.5, 33.1) * 1e-2;
}

float3 apply_volumetric_fog(inout float3 color, in float3 position_world_space)
{
    float3 start_position = cameraPositon.xyz;
	
    const float3 ray_vector = position_world_space - start_position;
    const float ray_length = length(ray_vector);
    const float3 ray_direction = ray_vector / ray_length;
	
    const float3 L = normalize(-lightDirection.xyz);
    const float LoV = max(0, dot(ray_direction, L));

    const float steps = 64;
    const float step_length = ray_length / steps;
    const float3 step = ray_direction * step_length;

	
#if 0
	const float4x4 dither_pattern =
	{
		{ 0.0f, 0.5f, 0.125f, 0.625f },
		{ 0.75f, 0.22f, 0.875f, 0.375f },
		{ 0.1875f, 0.6875f, 0.0625f, 0.5625 },
		{ 0.9375f, 0.4375f, 0.8125f, 0.3125 }
	};
	// Offset the start position
	start_position += dither_pattern[position_world_space.x % 4][position_world_space.y % 4];
#endif
	
    float3 current_position_world_space = start_position;
    float current_depth_view_space = 0;
    float accumulated_fog = 0;
	
	// Volumetric fog computation
    for (int i = 0; i < steps; i++)
    {
        float current_fog_amount = 0.0f;
		
        float noise = 1.0;
        const float3 wind_velocity = float3(1.0, 0.0, 0.0);
#if 0
		noise = (1.0 + snoise((current_position_world_space.xyz + wind_velocity * scene_data.time) * effect_data.noise_scale)) * 0.5;
#else
        noise = 0.5 * noise3D.Sample(samplerStates[LINEAR], (current_position_world_space.xyz + wind_velocity * elapsedTime) * noiseScale).x + 0.5;
#endif
		
        float lit_amount = calculate_shadows_and_lighting(current_depth_view_space, current_position_world_space);
		
		// Exponential height fog
        float b = fogHeightFalloff;
        float c = fogDensity * noise;
        float t = step_length * c * exp(-current_position_world_space.y * b);
        float vy = b * step.y * step_length;
        current_fog_amount = lit_amount * max(0.0, 1.0 - exp(t / vy * (exp(-vy) - 1.0)));
		
		// Mie and Layleigh Scattering
        const float scattering_intensity = 0.02;
        const float scattering = lit_amount * (compute_mie_scattering(LoV) /* + compute_layleigh_scattering(LoV)*/) * scattering_intensity;
		
        accumulated_fog += lerp(current_fog_amount, scattering, 0.5);
		
		// Extend the ray by a step in the ray direction
        current_position_world_space += step;
        current_depth_view_space += step_length;
    }

    const float max_fog_opacity = 0.75;
    accumulated_fog = min(max_fog_opacity, accumulated_fog);
	
    float3 fog_color = fogColor.rgb * fogColor.w;
#if 1
    float3 sun_color = float3(1.0, 0.9, 0.7) /* * scene_data.pure_white*/;
    float3 sun_direction = L;
    float sun_amount = max(0.0, dot(ray_direction, sun_direction));
    fog_color = lerp(fog_color, sun_color, pow(sun_amount, 128.0));
#endif

    return lerp(color, fog_color, saturate(accumulated_fog));
}

// トーンマップ
float3 JodieReinhardToneMap(float3 c)
{
    float l = dot(c, float3(0.2126, 0.7152, 0.0722));
    float3 tc = c / (c + 1.0);

    return lerp(c / (l + 1.0), tc, tc);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, number_of_level;
    colorTexture.GetDimensions(mipLevel, width, height, number_of_level);

    uint2 depth_map_dimensions;
    depthTexture.GetDimensions(mipLevel, depth_map_dimensions.x, depth_map_dimensions.y, number_of_level);


    // シーンからライティング済みのカラーテクスチャ
    float4 color = colorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);

    // シーンから深度値を取得
    float depth = depthTexture.SampleLevel(samplerStates[POINT], pin.texcoord, 0);

    // uv -> ndc 
    float4 positionNdc = CalculatedPositionNDC(pin);
    // ndc -> view 
    float4 positionViewSpace = mul(positionNdc, inverseProjection); // ndc → clip 
    positionViewSpace = positionViewSpace / positionViewSpace.w; // clip -> view 

    // ndc -> world 
    float4 position_world_space = mul(positionNdc, inverseViewProjection);

    // 影係数を計算
    int cascadeIndex = -1;
    float shadowFactor = CalculatedCascadedShadowFactor(pin, cascadeIndex);
    
    if (cascadeIndex > -1)
    {
        float3 layerColor = 1;
#if 1 //カスケードの色分け（デバッグ用）
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

            //float3 shadow= lerp(shadowColor, 1.0, shadowFactor) * layerColor;
            //return float4(shadow, 1);
        }
    }

#if 1
    // フォグの処理
    if (enableFog)
    {
        color.rgb = apply_volumetric_fog(color.rgb, position_world_space.xyz);

        float curr_depth = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).x;
        float4 sum = float4(0.0, 0.0, 0.0, 0.0);
        float4 sample;
        float radius = 4.0;
        float2 pos;
        float i, j;
        float sigma = 2.0 * radius * radius;
        float domain_gaussian = 0.0;
        float weight = 0.0;
        float distance = 0.0;
        float sample_depth;
        float range_gaussian;
        float sigma2 = 0.01;
        for (i = -radius; i <= radius; i += 1.0)
        {
            for (j = -radius; j <= radius; j += 1.0)
            {
                float dx = i / depth_map_dimensions.x;
                float dy = j / depth_map_dimensions.y;
                pos.x = pin.texcoord.x + dx;
                pos.y = pin.texcoord.y + dy;
                sample = fogTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pos);

                distance = i * i + j * j;
                domain_gaussian = exp(-distance / sigma);
			
                sample_depth = depthTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pos).x;
                distance = (curr_depth - sample_depth) * (curr_depth - sample_depth);
                range_gaussian = exp(-distance / sigma2);
			
                sum += sample * domain_gaussian * range_gaussian;
                weight += domain_gaussian * range_gaussian;
            }
        }
        float4 volumetric_light_color = sum / weight;
	
	//return volumetric_light_color;
	
        color.rgb = color.rgb * volumetric_light_color.a + volumetric_light_color.rgb;

        //float3 fogColor = CalculatedFogColor(pin);
        //color.rgb += fogColor;
        //return float4(fogColor, 1);
    }

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
#endif

    // SSAOの処理
    const float radius = 4.0;
    const float sigma = 2.0 * radius * radius;
    const float sigma2 = 0.01;

    float curr_depth = depth;
    float weight = 0.0;
	
    float accumulated_occlusion = 0;
	
    for (float i = -radius; i <= radius; i += 1.0)
    {
        for (float j = -radius; j <= radius; j += 1.0)
        {
            float dx = i / width;
            float dy = j / height;
            float2 uv = float2(pin.texcoord.x + dx, pin.texcoord.y + dy);
			
            float distance = i * i + j * j;
            float domain_gaussian = exp(-distance / sigma);
			
            float sample_depth = depthTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], uv, 0).x;
            distance = (curr_depth - sample_depth) * (curr_depth - sample_depth);
            float range_gaussian = exp(-distance / sigma2);
			
			// Sample occlusion(ambient) factor
            float sample_occlusion = ssaoTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], uv, 0).x;
            accumulated_occlusion += sample_occlusion * domain_gaussian * range_gaussian;

            weight += domain_gaussian * range_gaussian;
        }
    }
    float occlusion = accumulated_occlusion / weight;
    //if (pin.texcoord.x > split_u)
    {
        color *= occlusion;
    }

    // トーンマップ
    color.rgb = JodieReinhardToneMap(color.rgb);


    return color;
}