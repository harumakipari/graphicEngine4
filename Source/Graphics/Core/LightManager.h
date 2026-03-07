#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Graphics/Core/ConstantBuffer.h"

class Scene;

struct SharedLightParam
{
    DirectX::XMFLOAT4 color;
    float range;
};

class LightManager
{
public:
    struct PointLight
    {
        DirectX::XMFLOAT4 position{ 0.0f,0.0f,0.0f,0.0f };
        DirectX::XMFLOAT4 color{ 1.0f,0.0f,0.0f,0.0f };
        float range = 0.0f;
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

    std::shared_ptr<SharedLightParam> FindSharedLight(const std::string& name)
    {
        auto it = sharedLights.find(name);
        if (it == sharedLights.end())
            return nullptr;

        return it->second;
    }
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
        constants.lightDirection = dir;
        lightColor = color;
    }

    void DrawGUI();

    const DirectX::XMFLOAT4& GetLightDirection() const { return constants.lightDirection; }
private:
    //DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,3.8f };
    //float iblIntensity = 0.001f;  //Image Basesd Lightingの強度
    float iblIntensity = 1.01f;  //Image Basesd Lightingの強度


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

    std::unordered_map<std::string, std::shared_ptr<SharedLightParam>> sharedLights;
};
