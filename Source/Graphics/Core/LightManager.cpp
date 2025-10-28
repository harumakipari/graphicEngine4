#include "LightManager.h"

#include <string>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif


void LightManager::Initialize(ID3D11Device* device)
{
    int a = 0;
    _ASSERT_EXPR(device != nullptr, L"Device is null in LightManager::Initialize");
    lightCBuffer = std::make_unique<ConstantBuffer<LightConstants>>(device);
}

void LightManager::Update(float deltaTime)
{
    assert(pointLights.size() <= 8);
    constants.iblIntensity = iblIntensity;
    constants.directionalLightEnable = directionalLightEnable;
    constants.pointLightCount = pointLightCount;
    constants.lightColor = lightColor;
    constants.lightDirection = lightDirection;
    constants.directionalLightEnable = static_cast<int>(directionalLightEnable);
    constants.pointLightEnable = static_cast<int>(pointLightEnable);
    // デフォルト初期化
    for (int i = 0; i < 8; i++)
    {
        if (i < pointLights.size())
            constants.pointsLight[i] = pointLights[i];
        else
            constants.pointsLight[i] = {}; // 余分はゼロクリア
    }
}

void LightManager::Apply(ID3D11DeviceContext* immediateContext, int slot) const
{
    lightCBuffer->data = constants;
    lightCBuffer->Activate(immediateContext, slot);
}

void LightManager::DrawGUI()
{
#ifdef USE_IMGUI
    //ImGui::Checkbox("useDeferredRendering", &useDeferredRendering);
    ImGui::Checkbox("directionalLightEnable", &directionalLightEnable);
    ImGui::SliderFloat3("Light Direction", &lightDirection.x, -1.0f, 1.0f);
    ImGui::SliderFloat3("Light Color", &lightColor.x, -1.0f, 1.0f);
    ImGui::SliderFloat("IBL Intensity", &iblIntensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Light Intensity", &lightColor.w, 0.0f, 10.0f);
    ImGui::Checkbox("pointLightEnable", &pointLightEnable);
    ImGui::SliderInt("Point Light Count", &pointLightCount, 0, 8);
    if (pointLights.size() != static_cast<size_t>(pointLightCount))
        pointLights.resize(pointLightCount); // 個数を合わせる

    for (int i = 0; i < pointLightCount; i++)
    {
        std::string header = "PointLight[" + std::to_string(i) + "]";
        if (ImGui::CollapsingHeader(header.c_str()))
        {
            ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), &pointLights[i].position.x, 0.1f);
            ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), &pointLights[i].color.x);
            ImGui::SliderFloat(("Range##" + std::to_string(i)).c_str(), &pointLights[i].range, 0.0f, 10.0f);
            ImGui::SliderFloat(("Intensity##" + std::to_string(i)).c_str(), &pointLights[i].color.w, 0.0f, 10.0f);
        }
    }
#endif
}