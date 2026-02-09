#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Graphics/Core/ConstantBuffer.h"

class Scene;

class LightManager
{
public:
    struct PointLight
    {
        DirectX::XMFLOAT4 position{ 0.0f,0.0f,0.0f,0.0f };
        DirectX::XMFLOAT4 color{ 1.0f,0.0f,0.0f,1.0f };
        float range = 0.5f;
        float pads[3] = {};
    };

    struct LightConstants
    {
        DirectX::XMFLOAT4 lightDirection = {};
        DirectX::XMFLOAT4 lightColor = {};
        float iblIntensity = 1.0f;
        int directionalLightEnable = 1;// 平行光源の on / off
        int pointLightEnable = 1;
        int pointLightCount = 0;
        PointLight pointsLight[32];
    };

public:
    void Initialize(ID3D11Device* device);
    void Update(float deltaTime);
    void Apply(ID3D11DeviceContext* context, int slot) const;

    void AddPointLight(const PointLight& light)
    {
        renderPointLights.push_back(light);
    }

    void CollectPointLightsFromScene(const Scene& scene);

    void SetDirectionalLight(const DirectX::XMFLOAT4& dir, const DirectX::XMFLOAT4& color)
    {
        lightDirection = dir;
        lightColor = color;
    }

    void DrawGUI();

    const DirectX::XMFLOAT4& GetLightDirection() const { return lightDirection; }
private:
    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,3.8f };
    //float iblIntensity = 3.418f;  //Image Basesd Lightingの強度
    float iblIntensity = 0.001f;  //Image Basesd Lightingの強度

    DirectX::XMFLOAT4 pointLightPosition[8] =
    {
        { -2.0f,  2.0f, 0.0f, 10.0f },
        { -1.0f,  2.0f, 0.0f, 10.0f },
        { 0.0f,  2.0f, 0.0f, 10.0f },
        { 1.0f,  2.0f, 0.0f, 10.0f },
        { 2.0f,  2.0f, 0.0f, 10.0f },
        { 3.0f,  2.0f, 0.0f, 10.0f },
        { 4.0f,  2.0f, 0.0f, 10.0f },
        { 5.0f,  2.0f, 0.0f, 10.0f },
    };

    DirectX::XMFLOAT4 pointLightColor[8] =
    {
        { 1.0f, 0.0f, 0.0f, 10.0f },  // 赤
        { 0.0f, 1.0f, 0.0f, 10.0f },  // 緑
        { 0.0f, 0.0f, 1.0f, 10.0f },  // 青
        { 1.0f, 1.0f, 0.0f, 10.0f },  // 黄
        { 1.0f, 0.0f, 1.0f, 10.0f },  // マゼンタ
        { 0.0f, 1.0f, 1.0f, 10.0f },  // シアン
        { 1.0f, 0.5f, 0.0f, 10.0f },  // オレンジ
        { 0.5f, 0.0f, 1.0f, 10.0f },  // 紫
    };

    float pointLightRange[8] =
    {
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
        3.0f,
    };
    bool directionalLightEnable = true; // 平行光源の on / off
    bool pointLightEnable = true;
    int pointLightCount = 32;

    LightConstants constants = {};
    // GPUに送る最終のポイントライト情報
    std::vector<PointLight> renderPointLights;
    // ① デバッグ / 手動ライト（ImGui用）
    std::vector<PointLight> debugPointLights;
    // ② SceneComponent 由来ライト
    std::vector<PointLight> scenePointLights;

    std::unique_ptr<ConstantBuffer<LightConstants>> lightCBuffer;
};
