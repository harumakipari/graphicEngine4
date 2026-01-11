#include "pch.h"
#include "UIManager.h"
#include <imgui.h>


void UIManager::Update(float deltaTime)
{
    if (!enabled) return;
    mouseCaptured = false;
    for (auto ui : rootComponents)
    {
        if (ui->IsEnabled())
        {
            ui->UpdateTransform();
            ui->Update(deltaTime);
        }
    }

    // íœˆ—
    Cleanup();
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
