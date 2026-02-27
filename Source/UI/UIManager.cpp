#include "pch.h"
#include "UIManager.h"
#include <imgui.h>

#include "FontManager.h"


void UIManager::Update(float deltaTime)
{
    if (!enabled) return;

    if (!pendingAdd.empty())
    {
        for (auto& ui : pendingAdd)
        {
            rootComponents.push_back(ui);
        }
        pendingAdd.clear();
    }

    mouseCaptured = false;
    for (auto& ui : rootComponents)
    {
        if (!ui) continue;
        if (ui->IsEnabled())
        {
            ui->UpdateTransform();
            ui->Update(deltaTime);
        }
    }

    // íœˆ—
    Cleanup();
}

void UIManager::Draw(ID3D11DeviceContext* immediateContext)const
{
    if (!visible) return;

    // •`‰æ‡‚Éƒ\[ƒg zOrder‚ª‘å‚«‚¢‚Ù‚ÇŒã‚É•`‰æ‚³‚ê‚é
    std::vector<std::shared_ptr<UICoreComponent>> sortedComponents = rootComponents;
    std::sort(sortedComponents.begin(), sortedComponents.end(),
        [](const std::shared_ptr<UICoreComponent>& a, const std::shared_ptr<UICoreComponent>& b)
        {
            return a->zOrder < b->zOrder;
        });

    //FontManager::GetUIFont()->Begin(immediateContext);

    for (auto& ui : sortedComponents)
    {
        if (ui->IsVisible())
        {
            ui->Draw(immediateContext);
        }
    }

    //RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    //RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);
    //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);

    //FontManager::GetUIFont()->End(immediateContext);
}

void UIManager::DrawFont(ID3D11DeviceContext* immediateContext)const
{
    if (!visible) return;

    // •`‰æ‡‚Éƒ\[ƒg zOrder‚ª‘å‚«‚¢‚Ù‚ÇŒã‚É•`‰æ‚³‚ê‚é
    std::vector<std::shared_ptr<UICoreComponent>> sortedComponents = rootComponents;
    std::sort(sortedComponents.begin(), sortedComponents.end(),
        [](const std::shared_ptr<UICoreComponent>& a, const std::shared_ptr<UICoreComponent>& b)
        {
            return a->zOrder < b->zOrder;
        });


    FontManager::GetUIFont()->Begin(immediateContext);
    for (auto& ui : sortedComponents)
    {
        if (ui->IsVisible())
        {
            if (ui->IsVisible())
            {
                ui->DrawTexts(immediateContext);
                //ui->ClearDirty();
            }
        }
    }
    FontManager::GetUIFont()->End(immediateContext);

}

void UIManager::DrawSceneChangeSprite(ID3D11DeviceContext* immediateContext)const
{
    if (!visible) return;

    // •`‰æ‡‚Éƒ\[ƒg zOrder‚ª‘å‚«‚¢‚Ù‚ÇŒã‚É•`‰æ‚³‚ê‚é
    std::vector<std::shared_ptr<UICoreComponent>> sortedComponents = rootComponents;
    std::sort(sortedComponents.begin(), sortedComponents.end(),
        [](const std::shared_ptr<UICoreComponent>& a, const std::shared_ptr<UICoreComponent>& b)
        {
            return a->zOrder < b->zOrder;
        });


    FontManager::GetUIFont()->Begin(immediateContext);
    for (auto& ui : sortedComponents)
    {
        if (ui->IsVisible())
        {
            if (ui->IsVisible())
            {
                ui->DrawSceneChangeSprite(immediateContext);
                //ui->ClearDirty();
            }
        }
    }
    FontManager::GetUIFont()->End(immediateContext);

}

void UIManager::DrawImGUi()
{

    if (ImGui::Begin("UI Manager"))
    {
        ImGui::Checkbox("Visible", &visible);
        ImGui::Checkbox("Enabled", &enabled);

        ImGui::Separator();

        for (size_t i = 0; i < rootComponents.size(); ++i)
        {
            ImGui::PushID(static_cast<int>(i));
            rootComponents[i]->DrawImGui();
            ImGui::PopID();
        }

        if (ImGui::Button("Clear All"))
        {
            Clear();
        }
    }
    ImGui::End();
}

void UIManager::SetAllUIActive(bool visible, bool enabled)
{
    this->visible = visible;
    this->enabled = enabled;

    for (auto& ui : rootComponents)
    {
        if (auto sceneChange = std::dynamic_pointer_cast<UISceneChangeComponent>(ui))
        {
            continue;
        }
        if (!ui) continue;

        ui->SetVisible(visible);
        ui->SetEnable(enabled);
    }
}
