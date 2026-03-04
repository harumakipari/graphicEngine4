#include "pch.h"
#include "LightManager.h"
#include "Engine/Scene/Scene.h"
#include "Components/Render/PointLightComponent.h"
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

    // シャンデリアの共有ライト
    {
        sharedLights["MainChandelier"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.584078431f, 0.254152089f, 2.39999986f),
            10.0f
        };

        sharedLights["CandleChandelier"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.491020888f, 0.234550565f, 2.39999986f),
            1.5f
        };
    }

    // 燭台の共有ライト
    {
        sharedLights["TopCandelabra"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.577580452f, 0.309468925f, 2.0f),
            3.5f
        };

        sharedLights["SideCandelabra"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.577580452f, 0.309468925f, 1.19999993f),
            1.0f
        };
    }

    // 火鉢の共有ライト
    {
        sharedLights["BrazierCenterBig"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.533276379f, 0.258182853f, 1.43999994f),
            10.f
        };

        sharedLights["BrazierCenterSmall"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.533276379f, 0.258182853f, 1.5999999f),
            8.0f
        };
    }

    // 地面の火鉢の共有ライト
    {
        sharedLights["GroundBrazierLight"] =
        {
            DirectX::XMFLOAT4(1.0f, 0.577580452f, 0.258182883f, 2.39999986f),
            10.f
        };
    }
}

void LightManager::Update(float deltaTime)
{
    SetDirectionalLight(constants.lightDirection, lightColor);

    renderPointLights.clear();

    //// ① デバッグライト
    //for (auto& l : debugPointLights)
    //{
    //    renderPointLights.push_back(l);
    //    if (renderPointLights.size() >= 8) break;
    //}

    // ② Sceneライト
    for (auto& l : scenePointLights)
    {
        renderPointLights.push_back(l);
        if (renderPointLights.size() >= pointLightCount) break;
    }

    constants.pointLightCount = static_cast<int>(renderPointLights.size());
    constants.iblIntensity = iblIntensity;
    constants.directionalLightEnable = directionalLightEnable;
    constants.pointLightCount = pointLightCount;
    constants.lightColor = lightColor;
    //constants.lightDirection = lightDirection;
    constants.directionalLightEnable = static_cast<int>(directionalLightEnable);
    constants.pointLightEnable = static_cast<int>(pointLightEnable);
    // デフォルト初期化
#if 1
    for (int i = 0; i < pointLightCount; i++)
    {
        constants.pointsLight[i] =
            (i < renderPointLights.size()) ? renderPointLights[i] : PointLight{};
    }
#endif // 1
}

void LightManager::Apply(ID3D11DeviceContext* immediateContext, int slot) const
{
    lightCBuffer->data = constants;
    lightCBuffer->Activate(immediateContext, slot);
}

void LightManager::CollectPointLightsFromScene(const Scene& scene)
{
    scenePointLights.clear();

    for (auto& actor : scene.GetActorManager()->GetAllActors())
    {
        std::vector<PointLightComponent*> components;
        actor->GetComponents<PointLightComponent>(components);
        for (auto& light : components)
        {
            if (!light->IsUsePointLight()) continue;

            scenePointLights.push_back(light->ToRenderLight());
            if (scenePointLights.size() >= pointLightCount)
            {
                break;
            }
        }
    }
}

void LightManager::DrawGUI()
{
#ifdef USE_IMGUI
    //ImGui::Checkbox("useDeferredRendering", &useDeferredRendering);
    ImGui::Checkbox("directionalLightEnable", &directionalLightEnable);
    ImGui::SliderFloat3("Light Direction", &constants.lightDirection.x, -1.0f, 1.0f);
    ImGui::SliderFloat3("Light Color", &lightColor.x, -1.0f, 1.0f);
    ImGui::SliderFloat("IBL Intensity", &iblIntensity, 0.0f, 20.0f);
    ImGui::SliderFloat("Light Intensity", &lightColor.w, 0.0f, 20.0f);
    ImGui::Checkbox("pointLightEnable", &pointLightEnable);
    ImGui::SliderInt("Point Light Count", &pointLightCount, 0, 32);
    if (debugPointLights.size() != static_cast<size_t>(pointLightCount))
        debugPointLights.resize(pointLightCount); // 個数を合わせる

    if (ImGui::TreeNode(U8("共有ライト")))
    {
        for (auto& [name, light] : sharedLights)
        {
            if (ImGui::TreeNode(name.c_str()))
            {
                ImGui::ColorEdit3("Color", &light.color.x);
                ImGui::SliderFloat("Intensity", &light.color.w, 0.0f, 30.0f);
                ImGui::SliderFloat("Range", &light.range, 0.0f, 20.0f);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

#if 0
    for (int i = 0; i < pointLightCount; i++)
    {
        std::string header = "PointLight[" + std::to_string(i) + "]";
        if (ImGui::CollapsingHeader(header.c_str()))
        {
            ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), &debugPointLights[i].position.x, 0.1f);
            ImGui::ColorEdit3(("Color##" + std::to_string(i)).c_str(), &debugPointLights[i].color.x);
            ImGui::SliderFloat(("Range##" + std::to_string(i)).c_str(), &debugPointLights[i].range, 0.0f, 10.0f);
            ImGui::SliderFloat(("Intensity##" + std::to_string(i)).c_str(), &debugPointLights[i].color.w, 0.0f, 10.0f);
        }
    }

#endif // 0
#endif
}