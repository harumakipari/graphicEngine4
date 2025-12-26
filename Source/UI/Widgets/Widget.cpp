#include "pch.h"
#include "Widget.h"

#include <imgui.h>

void UICoreComponent::DrawImGui()
{
#ifdef USE_IMGUI
    ImGui::Text("Name: %s", name.c_str());
    ImGui::Checkbox("Visible", &visible);
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat2("Position", &position.x, 1.0f);
    ImGui::DragFloat2("Size", &size.x, 1.0f);
#endif
}
