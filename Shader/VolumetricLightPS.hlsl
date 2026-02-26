#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Lights.hlsli"
#include "Sampler.hlsli"

Texture2D depthMap : register(t0);
Texture2DArray shadowMap : register(t1);

Texture2D noise2dMap : register(t30);
Texture3D noise3dMap : register(t31);

// Zバッファから線形0..1深度へ
inline float Linear01Depth(float z, float near, float far)
{
    return 1.0 / ((1 - far / near) * z + (far / near));
}
float MieScattering(float cosAngle, float g)
{
    return (1.0 / (4.0 * 3.14159265358979)) * ((1 - (g * g)) / (pow((1 + (g * g)) - (2 * g) * cosAngle, 1.5)));
}
void ApplyHeightFog(float3 position /*world_space*/, inout float density)
{
    const float heightScale = fogHeightFalloff;
    density *= exp(-(position.y - groundLevel) * heightScale);
}

float GetLightAttenuation(float3 positionWorldSpace)
{
    float depth = length(positionWorldSpace - cameraPositon.xyz);
	
    int cascadeIndex = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = cascadedPlaneDistances[layer];
        if (distance > depth)
        {
            cascadeIndex = layer;
            break;
        }
    }
    if (cascadeIndex == -1)
    {
        return 1;
    }
    float4 positionLightSpace = mul(float4(positionWorldSpace, 1.0), cascadedMatrices[cascadeIndex]);
    positionLightSpace /= positionLightSpace.w;

	// To texture space
    positionLightSpace.x = positionLightSpace.x * +0.5 + 0.5;
    positionLightSpace.y = positionLightSpace.y * -0.5 + 0.5;
	
    float atten = shadowMap.SampleCmpLevelZero(comparisionSamplerState, float3(positionLightSpace.xy, cascadeIndex), positionLightSpace.z - shadowDepthBias).x;
	
#if 0
	const float shadow_strength = 0.2;
	atten = shadow_strength + atten * (1 - shadow_strength);
#endif	
    return atten;
}
float4 DitheredRayMarch(float2 screenPos, float3 rayStart, float3 rayDir, float rayLength)
{
#if 1
    const float4x4 ditherPattern =
    {
        { 0.0f, 0.5f, 0.125f, 0.625f },
        { 0.75f, 0.22f, 0.875f, 0.375f },
        { 0.1875f, 0.6875f, 0.0625f, 0.5625 },
        { 0.9375f, 0.4375f, 0.8125f, 0.3125 }
    };
    float ditherValue = ditherPattern[screenPos.x % 4][screenPos.y % 4];
#else
	float ditherValue = 0;
#endif
	
    const int stepCount = 16;

    float stepSize = rayLength / stepCount;
    float3 step = rayDir * stepSize;
	
    float3 currentPosition = rayStart + step * ditherValue;

    float4 vlight = 0;

    float extinction = 0;
    float cosAngle = dot(normalize(lightDirection.xyz), -rayDir);
	
	[loop]
    for (int i = 0; i < stepCount; ++i)
    {
        float atten = GetLightAttenuation(currentPosition);
		
		//float density = get_density(currentPosition);
        float density = 1;
#if 1
        const float time = elapsedTime * timeScale;

        const float3 noiseVelocity = normalize(float3(1, 0, 0));
		
        float3 position = frac(currentPosition * noiseScale + noiseVelocity * time);
        float noise = 0.5 * noise3dMap.Sample(samplerStates[LINEAR], position).x + 0.5;
        const float sharpnessFactor = 1.0;
        noise = pow(noise, sharpnessFactor);
        const float noiseIntensityOffset = 0.2;
        const float noiseIntensity = fogDensity;
        density = max(0, noise - noiseIntensityOffset) * noiseIntensity;
#endif
        ApplyHeightFog(currentPosition, density);

        const float scatteringCoef = 0.815f;
        const float extinctionCoef = 0.0031f;
        float scattering = scatteringCoef * stepSize * density;
        extinction += extinctionCoef * stepSize * density;

        float4 light = atten * scattering * exp(-extinction);
        vlight += light;

        currentPosition += step;
    }

#if 1
	// 指向性ライトに位相関数を適用する
    const float mieG = mieScatteringCoef;
    vlight *= MieScattering(cosAngle, mieG);
#endif

	// 光の色を適用する
    vlight.rgb *= fogColor.rgb * fogColor.a;

    vlight = max(0, vlight);
#if 1 
    vlight.w = exp(-extinction);
#endif
	
    return saturate(vlight);
}

float4 main(VS_OUT pin) : SV_TARGET
{
#if 0
	float depth = depthMap.Sample(sampler_states[POINT_WRAP], pin.texcoord).x;
#else
	// 深度ダウンサンプリング
    float4 sampledDepth = depthMap.Gather(samplerStates[POINT], pin.texcoord);
    float minDepth = min(min(sampledDepth.x, sampledDepth.y), min(sampledDepth.z, sampledDepth.w));
    float maxDepth = max(max(sampledDepth.x, sampledDepth.y), max(sampledDepth.z, sampledDepth.w));

    // チェス盤のパターンで近い方の深度を選択
    int2 position = pin.position.xy % 2;
    int index = position.x + position.y;
    float depth = index == 1 ? minDepth : maxDepth;
#endif
	
    float4 positionNdc;
	// テクスチャ空間からNDCへ
    positionNdc.x = pin.texcoord.x * +2 - 1;
    positionNdc.y = pin.texcoord.y * -2 + 1;
    positionNdc.z = depth;
    positionNdc.w = 1;

	// ndcからワールド空間へ
    float4 positionWorldSpace = mul(positionNdc,inverseViewProjection);
    positionWorldSpace = positionWorldSpace / positionWorldSpace.w;
	
    float3 rayStart = cameraPositon.xyz;
    float3 rayDir = positionWorldSpace.xyz - cameraPositon.xyz;
	
#if 0
	// 遠近投影行列から近距離値と遠距離値を抽出する
	float near = -projection._43 / projection._33;
	float far = -projection._33 / (1 - projection._33) * near;
	float linearDepth = Linear01Depth(depth, near, far);
	rayDir *= linearDepth;
#endif
    float rayLength = length(rayDir);
    rayDir /= rayLength;
	
#if 1
    const float maxRayLength = fogCutoffDistance;
    rayLength = min(rayLength, maxRayLength);
#endif
	
    float4 color = DitheredRayMarch(pin.position.xy, rayStart, rayDir, rayLength);
	
#if 0
	if (linearDepth > 0.999999)
	{
		const float skyboxExtinctionCoef = 0.9;
		color.w = lerp(color.w, 1, 1.0f - skyboxExtinctionCoef);
	}
#endif
	
	//color.a = depth;
	
    return color;
}
