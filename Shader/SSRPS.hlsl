#include "Sampler.hlsli"
#include "FullScreenQuad.hlsli"

Texture2D positionTexture : register(t0); // ワールド空間
Texture2D normalTexture : register(t1); // ワールド空間
Texture2D colorTexture : register(t2);

cbuffer VIEW_CONSTANTS_BUFFER : register(b4)
{
    row_major float4x4 viewProjection;
    float4 cameraPositon;
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 inverseProjection;
    row_major float4x4 inverseViewProjection;
    row_major float4x4 invView;
}

cbuffer SSR_CONSTANTS_BUFFER : register(b5)
{
    float reflectionIntensity; // 反射の強さ
    float maxDistance; // レイをどこまで飛ばすか
    float resolution; // レイのステップ密度
    int steps; // 二分探索の回数
    float thickness; // ヒット判定の厚み
}

float2 NdcToUv(float2 ndc)
{
    float2 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    return uv;
}

inline float FSchlick(float f0, float cos)
{
    return f0 + (1 - f0) * pow(1 - cos, 5);
}

float3 main(VS_OUT pin) : SV_TARGET
{
    int steps = 10;

    uint2 dimensions;
    uint mipLevel = 0, numberOfLevels;
    positionTexture.GetDimensions(mipLevel, dimensions.x, dimensions.y, numberOfLevels);
    
    float4 position = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], pin.texcoord); // world空間
    float3 normal = normalTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).xyz; // world空間

    float4 positionFrom = position;
    float4 positionTo = positionFrom;

    // 視線ベクトル
    float3 incident = normalize(position.xyz - cameraPositon.xyz);

    // 反射ベクトル
    float3 reflection = normalize(reflect(incident, normal.xyz));

    // ワールド空間でレイを定義する
    float4 startWorld = float4(positionFrom.xyz + (reflection * 0), 1);
    float4 endWorld = float4(positionFrom.xyz + (reflection * maxDistance), 1);

    if (endWorld.z < 0)
    { // 画面外に行かないように補正
        float3 v = endWorld.xyz - startWorld.xyz;
        endWorld.xyz = startWorld.xyz + v * abs(startWorld.z / v.z);
    }

    // ワールド空間 -> スクリーン空間
    float4 startFrag = mul(startWorld, viewProjection); //　ワールド空間 -> クリップ空間
    startFrag /= startFrag.w; // クリップ空間 -> ndc 空間
    startFrag.xy = NdcToUv(startFrag.xy); // ndc 空間 -> テクスチャ空間
    startFrag.xy *= dimensions;
    
    // ワールド空間 -> スクリーン空間
    float4 endFrag = mul(endWorld, viewProjection); //　ワールド空間 -> クリップ空間
    endFrag /= endFrag.w; // クリップ空間 -> ndc 空間
    endFrag.xy = NdcToUv(endFrag.xy); // ndc 空間 -> テクスチャ空間
    endFrag.xy *= dimensions;
    
    float2 frag = startFrag.xy;
    
    float4 uv = 0;
    uv.xy = frag / dimensions;

    // スクリーン空間のレイを設定する
    float deltaX = endFrag.x - startFrag.x;
    float deltaY = endFrag.y - startFrag.y;
    
    float useX = abs(deltaX) >= abs(deltaY) ? 1 : 0;
    float delta = lerp(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);

    // 1ステップで何ピクセル進むか
    float2 increment = float2(deltaX, deltaY) / max(delta, 0.001);
    
    float search0 = 0;
    float search1 = 0;
    
    int hit0 = 0;
    int hit1 = 0;
    
    float viewDistance = startWorld.z;
    float depth = thickness;
    
#define MAX_DELTA 64
    delta = min(MAX_DELTA, delta);
    [unroll(MAX_DELTA)] // レイマーチ
    for (int i = 0; i < (int) delta; ++i)
    {
        frag += increment;
        uv.xy = frag / dimensions;
        if (uv.x <= 0 || uv.x >= 1 || uv.y <= 0 || uv.y >= 1)
        {
            hit0 = 0;
            break;
        }

        positionTo = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy); // ワールド空間
        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), viewProjection);
        positionToClip /= positionToClip.w;

        search1 = lerp((frag.y - startFrag.y) / deltaY, (frag.x - startFrag.x) / deltaX, useX);
        search1 = clamp(search1, 0.0, 1.0);

        // Perspective Correct Interpolation
        // NDC.z ベースで比較する
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        // 深度比較
        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
#if 1
        // ヒット判定
        if (depth > 0 && depth < thickness)
        {// レイが GBuffer の位置より奥で thickness 以内
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#else
        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#endif
    }
#if 0
    hit1 = hit0;
#else
    search1 = search0 + ((search1 - search0) / 2.0);
    steps *= hit0;
    
    [unroll]    // 二分探索（ヒット精密化）
    for (i = 0; i < steps; ++i)
    {
        frag = lerp(startFrag.xy, endFrag.xy, search1);
        uv.xy = frag / dimensions;
        
        positionTo = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy); //viewSpace
        positionTo = mul(float4(positionTo.xyz, 1.0), view);

        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), projection);
        positionToClip /= positionToClip.w;
        
        // PerspectiveCorrect Interpolation
#if 1
        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
        
        if (depth > 0 && depth < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#else
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#endif
    }
#endif
    float visibility = hit1;
    visibility *= (1 - max(dot(-normalize(positionFrom.xyz), reflection), 0));
    visibility *= (1 - clamp(depth / thickness, 0, 1));
    visibility *= positionTo.w;
    visibility *= (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) ? 0 : 1;
    visibility = clamp(visibility, 0, 1);

    // フレネル＋色取得
    float fresnel = saturate(FSchlick(0.04, max(0, dot(reflection, normal.xyz))));
    float3 reflectionColor = colorTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy).rgb;
    reflectionColor = fresnel * reflectionColor * visibility * reflectionIntensity;
    return reflectionColor;
}