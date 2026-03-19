#pragma once

struct SceneLightConstants
{
    DirectX::XMFLOAT4 lightDirection = { 0.722f, -0.38f, -0.0211f, 0.9f };// w:attenuation Rate
    DirectX::XMFLOAT4 lightColor = { 1.0f, 0.8f, 1.0f, 2.6f }; //w colorPower

    float iblIntensity = 2.4f;
    int directionalLightEnable = 1;// 平行光源の on / off
    int pointLightEnable = 1;
    int pointLightCount = 40;

    DirectX::XMFLOAT3 rimColor = { 0.3f,0.5f,1.0f };
    float rimIntensity = 0.112f;

    float rimPower = 3.0f;
    float kc = 1.0f;
    float kl = 0.7f;
    float kq = 1.8f;

    float diffuseIntensity = 1.272f;
    float specularIntensity = 0.323f;
    float pointLightDiffuseIntensity = 0.9f;
    float pointLightSpecularIntensity = 0.15f;
};

struct SceneShaderConstants
{
    float shadowColor = 0.75f;
    float shadowDepthBias = -0.00207f;
    float splitU = 0.0f;
    float	hueShift = -0.028f;	// 色相調整 -1 から 1 （-1 は負方向の 180 度、0 は変更なし、1 は正方向の 180 度）

    float	saturation = 0.02f;	// 彩度調整（-1は濃灰、0は変化なし、1は最大彩度）
    float	brightness = 0.013f;	// 明度調整（-1 は完全な黒、0 は変化なし、1 は完全な白）
    float	contrast = 0.145f;	// コントラスト調整（-1は完全な灰色、0は変化なし、1は最大コントラスト）
    float focusDistance = 4.6f; // 焦点距離

    float dofRange = 80.0f;  // 被写界深度範囲
    float objectIblIntensity = 23.0f; // オブジェクトのIblIntensity (今は骸骨を明るくするために)
    int renderStep = 0; // デバック表示用のレンダーステップ
    int enableToneMapping = 1; // トーンマッピング有効化フラグ

    int enableSsao = 1;
    int enableCascadedShadowMaps = 1;
    int enableSsr = 1;
    int enableFog = 1;

    int enableBloom = 1;
    int enableBlur = 1;
    int enableDof = 0;
    int colorizeCascadedLayer = 0;
};


struct CascadedShadowMapConstants
{
    float criticalDepthValue = 247.0f;
    float splitSchemeWeight = 0.83f;// 1.0 に近いほど対数分割寄り
    bool fitToCascade = true;// true: カスケード毎にnearを変える
    float zDepthScale = 40.4f;// Z拡張倍率（シャドウ欠け防止）
};

struct FogConstants
{
    DirectX::XMFLOAT4 fogColor = { 0.63f,0.63f, 0.68f, 0.6f }; // w: fog intensity

    float fogDensity = 0.24f;
    float fogHeightFalloff = 0.027f;
    float groundLevel = 28.0f;
    float fogCutoffDistance = 30.0f;

    float mieScatteringFactor = 0.55f;
    float timeScale = 0.9f;
    float noiseScale = 0.5f;
    int enableDither = 1;

    float globalFogIntensity = 0.034f;// 全体にフォグをどれくらいかけるか
};

struct SSRConstantBuffer
{
    float reflectionIntensity = 0.8f;
    float maxDistance = 1.3f;
    float resolution = 0.25f;
    int steps = 5;
    float thickness = 0.38f;
};

struct SSAOConstantBuffer
{
    float radius = 0.2f;
    float bias = 0.15f;
    float power = 0.02f;
};

struct BloomConstantBuffer
{
    float bloomExtractionThreshold = 5.0;
    float bloomIntensity = 0.335f;
};

class SceneSettings // 今の状態
{
public:
    SceneLightConstants sceneLightConstants{};
    SceneShaderConstants sceneShaderConstants{};
    CascadedShadowMapConstants cascadedShadowMapConstants{};
    FogConstants fogConstants{};
    SSRConstantBuffer ssrConstantBuffer{};
    SSAOConstantBuffer ssaoConstantBuffer{};
    BloomConstantBuffer bloomConstantBuffer{};
};