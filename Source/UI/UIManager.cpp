#include "pch.h"
#include "UIManager.h"
#include <imgui.h>

void UIManager::DrawImGUi()
{

    if (ImGui::Begin("UI Manager"))
    {
        ImGui::Checkbox("Visible", &visible);
        ImGui::Checkbox("Enabled", &enabled);

        ImGui::Separator();

        for (size_t i = 0; i < rootComponents.size(); ++i)
        {
            auto& ui = rootComponents[i];
            ImGui::PushID((int)i);

            ImGui::Checkbox("Visible", &ui->visible);
            ImGui::SameLine();
            ImGui::Checkbox("Enabled", &ui->enabled);

            ImGui::Text("Pos(%.1f, %.1f)", ui->position.x, ui->position.y);
            ImGui::DragFloat2("Position", &ui->position.x, 1.0f);
            ImGui::DragFloat2("Size", &ui->size.x, 1.0f);

            if (auto gauge = std::dynamic_pointer_cast<UIGaugeComponent>(ui))
            {
                ImGui::SliderFloat("Value", &gauge->value, 0.0f, 1.0f);
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button("Clear All"))
        {
            Clear();
        }
    }
    ImGui::End();
}
