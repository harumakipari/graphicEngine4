#include "GltfModel.hlsli"
#include "Lights.hlsli"
#include "Sampler.hlsli"

Texture2D<float3> normalTexture : register(t12); // 水面法線マップ
Texture2D sceneColorTexture : register(t25); // 透明以外のライティング後のテクスチャ

float3 SafeNormalize(float3 v)
{
    float len = length(v);
    return len > 1e-6 ? v / len : float3(0, 0, 1);
}

half3 RGBToHSV(half3 c)
{
    half4 K = half4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    half4 p = lerp(half4(c.bg, K.wz), half4(c.gb, K.xy), step(c.b, c.g));
    half4 q = lerp(half4(p.xyw, c.r), half4(c.r, p.yzx), step(p.x, c.r));
    half d = q.x - min(q.w, q.y);
    half e = 1e-10;
    return half3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

half3 HSVToRGB(half3 c)
{
    half4 K = half4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    half3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
}

half4 HSVLerp_half(half4 A, half4 B, half t)
{
    A.xyz = RGBToHSV(A.xyz);
    B.xyz = RGBToHSV(B.xyz);

    // Hue 補正
    half d = B.x - A.x;
    if (d > 0.5)
        A.x += 1.0;
    half3 hsv = half3(A.x + d * t, A.y + (B.y - A.y) * t, A.z + (B.z - A.z) * t);
    hsv.x = frac(hsv.x);

    half3 rgb = HSVToRGB(hsv);
    return half4(rgb, A.w + t * (B.w - A.w));
}

// 3つを重み付きでブレンド（合成版）
float3 BlendThreeNormals(float3 n1, float3 n2, float3 n3, float3 w)
{
    // 単純合成後正規化（重みは合計1推奨）
    float3 sum = n1 * w.x + n2 * w.y + n3 * w.z;
    return SafeNormalize(sum);
}

// 小さな回転（Z回転のみ：水面法線の微回転に使える）
float3 RotateNormalZ(float3 n, float angle)
{
    float s = sin(angle), c = cos(angle);
    float3x3 rot = float3x3(c, -s, 0, s, c, 0, 0, 0, 1);
    return mul(rot, n);
}


// 簡易ノイズラッパ（ユーザーの Noise 関数があればそれを使う）
// ここでは Noise(pos) が 0..1 を返す想定
float2 Noise2(float2 uv)
{
    // 既存の Noise(x) があればそっちを呼ぶ:
    // return float2(Noise(uv), Noise(uv.yx+123.4));
    // 無ければ簡易ハッシュノイズ（deterministic）
    float n1 = frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
    float n2 = frac(sin(dot(uv + 37.0, float2(26.651, 95.467))) * 24634.6345);
    return float2(n1, n2);
}

// UV をランダム化（小さな揺らぎ）
float2 RandomizeUV(float2 uv, float strength, float speed)
{
    float2 n = Noise2(uv * 3.1 + float2(elapsedTime * 0.2 * speed, -elapsedTime * 0.15 * speed));
    return uv + (n - 0.5) * strength;
}

// Tangent-space gbuffer1Normal map (0..1) -> -1..1 and normalized
float3 DecodeNormal(float3 n)
{
    return SafeNormalize(n * 2.0 - 1.0);
}

float LightingSpecular(float3 L, float3 N, float3 V, float smoothness)
{
    float3 H = SafeNormalize(L + V);
    float NdotH = saturate(dot(N, H));
    return pow(NdotH, smoothness);
}

float GetMainSpecular(float3 normalWS, float3 viewWS, float smoothness)
{
    // smoothness をシェーダグラフと同様に拡張
    float s = exp2(10.0 * smoothness + 1.0);

    float3 Nn = normalize(normalWS);
    float3 Vn = SafeNormalize(viewWS);

    // mainLightDirection は「ライトが向いている方向（単位ベクトル）」として想定
    // もし your engine が -lightDirection で来ているなら符号を合わせて
    float3 L = normalize(-lightDirection.xyz); // 例: lightDirection が「太陽ベクトル」なら - にする
    return LightingSpecular(L, Nn, Vn, s);
}

float3 GetBlendedWaterNormal(
    float2 uv,
    float normalScale,
    float normalSpeed,
    float normalStrength,
    float3x3 TBN,
    bool useRotationBlend, // optional
    float uvRandomStrength, // optional: RandomizeUV 強さ（例 0.02..0.06）
    float uvRandomSpeed // optional: RandomizeUV 速度倍率
)
{
    // パラメータ デフォルト処理
    if (uvRandomStrength <= 0)
        uvRandomStrength = 0.03;
    if (uvRandomSpeed <= 0)
        uvRandomSpeed = 1.0;

    // --- 1) UV を少しランダム化してサンプル分散を作る ---
    float2 uvBase = uv * normalScale;
    float2 uvRnd1 = RandomizeUV(uvBase, uvRandomStrength, uvRandomSpeed);
    float2 uvRnd2 = RandomizeUV(uvBase * 1.07, uvRandomStrength * 0.9, uvRandomSpeed * 1.1);
    float2 uvRnd3 = RandomizeUV(uvBase * 0.93, uvRandomStrength * 1.1, uvRandomSpeed * 0.8);

    // --- 2) 時間オフセットでパン（speed） ---
    uvRnd1 += float2(elapsedTime * normalSpeed, -elapsedTime * normalSpeed * 0.6);
    uvRnd2 += float2(-elapsedTime * normalSpeed * 0.9, elapsedTime * normalSpeed * 0.8);
    uvRnd3 += float2(elapsedTime * normalSpeed * 0.4, elapsedTime * normalSpeed * -0.3);

    // --- 3) ノーマルテクスチャサンプル（Decode） ---
    float3 n1 = DecodeNormal(normalTexture.Sample(samplerStates[LINEAR], uvRnd1).xyz);
    float3 n2 = DecodeNormal(normalTexture.Sample(samplerStates[LINEAR], uvRnd2).xyz);
    float3 n3 = DecodeNormal(normalTexture.Sample(samplerStates[LINEAR], uvRnd3).xyz);

    // --- 4) optional: 微回転でさらに差をつける ---
    if (useRotationBlend)
    {
        // 回転角は UV ごとのノイズから得る（小さく）
        float a1 = (Noise2(uvRnd1).x - 0.5) * 0.8; // -0.4..0.4 rad
        float a2 = (Noise2(uvRnd2).x - 0.5) * 0.6;
        float a3 = (Noise2(uvRnd3).x - 0.5) * 0.5;
        n1 = RotateNormalZ(n1, a1);
        n2 = RotateNormalZ(n2, a2);
        n3 = RotateNormalZ(n3, a3);
    }

    // --- 5) 合成（重みは自由に調整） ---
    float3 weights = float3(0.45, 0.35, 0.20);
    float3 blendedTS = BlendThreeNormals(n1, n2, n3, weights);

    // --- 6) 強さを適用（0 -> flat, 1 -> full gbuffer1Normal） ---
    blendedTS = SafeNormalize(lerp(float3(0, 0, 1), blendedTS, saturate(normalStrength)));

    // --- 7) Tangent->World へ変換して返す ---
    float3 normalWS = mul(blendedTS, TBN);
    return SafeNormalize(normalWS);
}

// ---- Additional lights (simple single-point fallback) ----
float3 GetAdditionalSpecular(float3 normalWS, float3 positionWS, float3 viewWS, float smoothness, float hardness)
{
    float3 accum = float3(0, 0, 0);
    float s = exp2(10.0 * smoothness + 1.0);

    float3 Nn = normalize(normalWS);
    float3 Vn = SafeNormalize(viewWS);

    // --------------------
    // --- OPTION A: your engine の追加ライト API があればループして使う（疑似）
    //int pixelLightCount = GetAdditionalLightsCount();
    //for (int i = 0; i < pixelLightCount; ++i) {
    //    Light light = GetAdditionalLight(i, positionWS);
    //    float3 L = normalize(light.direction);
    //    float3 atten = light.gbuffer3Color * light.distanceAttenuation * light.shadowAttenuation;
    //    float spec_soft = LightingSpecular(L, Nn, Vn, s);
    //    float spec_hard = smoothstep(0.005, 0.01, spec_soft);
    //    float spec_term = lerp(spec_soft, spec_hard, hardness);
    //    accum += spec_term * atten;
    //}
    // --------------------

    // --------------------
    // --- OPTION B: 単一点光のフォールバック（エンジンに合わせて値を入れてください）
    // pointLightPos, pointLightColor, pointLightRange はアプリ側で用意
#if 0
        float3 Ldir = pointLightPos - positionWS;
        float dist = length(Ldir);
        Ldir = (dist > 1e-6) ? Ldir / dist : float3(0,0,1);
        float att = saturate(1.0 - dist / pointLightRange); // simple linear att
        float spec_soft = LightingSpecular(Ldir, Nn, Vn, s);
        float spec_hard = smoothstep(0.005, 0.01, spec_soft);
        float spec_term = lerp(spec_soft, spec_hard, hardness);
        accum += spec_term * pointLightColor * att;
#endif
    // --------------------

    return accum;
}

cbuffer ODEN_SOUP_CONSTANTS_BUFFER : register(b12)
{
    float normalScale = 7.36f;
    float normalStrength = 2.11f;
    float normalSpeed = 0.56f;
    float specularSmoothness = 0.229f;

    float3 mainLightColor = { 0.3, 0.3, 0.3 };
    float specularHardness = 0.215f; // 0..1 slider to blend soft->hard

    float3 specularColor = { 0.3, 0.3, 0.3 };
    float waterAlpha = 0.8;

    float4 shallowColor = float4(0.70, 0.90, 1.00, 1.0); // 明るい浅瀬

    float4 deepColor = float4(0.05, 0.28, 0.65, 1.0);

    float specularIntensity = 5.74f; // 全体スケール
    float turbidity; // 濁り
    float oilStrength; // 油膜
}


float4 main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET0
{
    float2 screenSize;
    sceneColorTexture.GetDimensions(screenSize.x, screenSize.y);
    float2 uv = pin.position.xy / screenSize;

   // 鍋の液面Y world space
    float soupSurfaceY = 0.9;

    // 深さ（0 = 表面）
    float soupDepth = saturate((soupSurfaceY - pin.wPosition.y) / 5.0);

    float depthFade = soupDepth;

    //return float4(depthFade.xxx, 1.0);

    // シーンカラー取得
    float3 sceneColor = sceneColorTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv).rgb;

    // ===== 水面法線取得 =====
    float3 N = normalize(pin.wNormal.xyz);
    float3 V = normalize(cameraPositon.xyz - pin.wPosition.xyz);
    float3 L = normalize(-lightDirection.xyz);

    float3 T = hasTangent ? normalize(pin.wTangent.xyz) : float3(1, 0, 0);
    float sigma = hasTangent ? pin.wTangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    float3x3 TBN = float3x3(T, B, N);
    float3 blendNormal = GetBlendedWaterNormal(uv, normalScale, normalSpeed, normalStrength, TBN, true, 0.04, 1.0);

    float3 worldPos = pin.wPosition.xyz;
    
    float3 viewWS = normalize(cameraPositon.xyz - worldPos); // world-space view dir

    float mainSpec = GetMainSpecular(blendNormal, viewWS, specularSmoothness);

    // optional: harden the main specular like a Step node (step threshold can be a parameter)
    //float mainSpecHard = smoothstep(specularStepLow, specularStepHigh, mainSpec); // e.g. 0.02..0.05
    float mainSpecHard = smoothstep(0.02, 0.05, mainSpec); // e.g. 0.02..0.05
    float mainSpecCombined = lerp(mainSpec, mainSpecHard, specularHardness);

    // additional
    float3 addSpec = GetAdditionalSpecular(blendNormal, worldPos, viewWS, specularSmoothness, specularHardness);

    // combine
    float3 totalSpecular = (mainSpecCombined * mainLightColor /*.xxx*/) + addSpec; // mainLightColor は float3
    float3 specularTerm = specularColor * totalSpecular * specularIntensity;


    float4 waterColor = HSVLerp_half(deepColor, shallowColor, depthFade);
    float alphaRaw = depthFade;
    waterColor.a = lerp(0.3, waterAlpha, alphaRaw);

    waterColor.a = waterAlpha; // 透明度（調整）


    // 最終色に加算（「水色に足す」方式）         ここに反射の
    float3 litColor = waterColor.rgb + specularTerm;

    float3 baseColor = sceneColor * (1.0 - waterColor.a) + litColor * waterColor.a; 

    return float4(baseColor, 1.0);
    return float4(litColor, waterColor.a);

}