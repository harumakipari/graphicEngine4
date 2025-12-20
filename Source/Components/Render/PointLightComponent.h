#pragma once
#include "Components/Base/SceneComponent.h"
#include "Graphics/Core/LightManager.h"

class PointLightComponent :public SceneComponent
{
public:
    PointLightComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner) {}

    virtual ~PointLightComponent() = default;

    virtual void Tick(float deltaTime)override
    {
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + " point Light").c_str()))
        {
            ImGui::Checkbox("enable", &enable);
            ImGui::SliderFloat("range", &range, 0.0f, +10.0f);
            ImGui::ColorEdit3("Color", &color.x);
            ImGui::SliderFloat("Intensity##", &color.w, 0.0f, 30.0f);
            ImGui::TreePop();
        }
#endif
    }

    LightManager::PointLight ToRenderLight() const
    {
        LightManager::PointLight l{};
        l.position = {
            GetComponentLocation().x,
                        GetComponentLocation().y,
                        GetComponentLocation().z,
                       1.0f };
        l.color = color;
        l.range = range;
        return l;
    }

    void SetColor(const DirectX::XMFLOAT3& color)
    {
        this->color.x = color.x ;
        this->color.y = color.y;
        this->color.z = color.z;
    }

    void SetRange(float range)
    {
        this->range = range;
    }

    void SetIntensity(float intensity)
    {
        this->color.w = intensity;
    }

    bool  IsUsePointLight()const { return enable; }

private:
    DirectX::XMFLOAT4 color{ 1,1,1,1 };
    float range = 3.0f;
    bool enable = true;

};
