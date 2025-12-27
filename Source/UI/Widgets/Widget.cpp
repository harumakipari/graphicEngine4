#include "pch.h"
#include "Widget.h"

#include <imgui.h>

void UICoreComponent::DrawImGui()
{
#ifdef USE_IMGUI
    ImGui::Text("Name: %s", name.c_str());
    ImGui::Checkbox("Visible", &visible);
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat2("worldPosition", &worldPosition.x, 1.0f);
    ImGui::DragFloat2("Size", &size.x, 1.0f);
    ImGui::DragFloat2("Scale", &scale.x, 0.2f);
    ImGui::SliderFloat("worldAngle", &worldAngle,0.0f,360.0f);
    ImGui::SliderFloat2("pivot", &pivot.x, 0.0f,1.0f);

#endif
}

void UICoreComponent::SetParent(UICoreComponent* newParent)
{
    if (parent == newParent) return;

    // Šù‘¶‚Ìe‚©‚çŠO‚·
    if (parent)
    {
        auto& siblings = parent->children;
        siblings.erase(
            std::remove(siblings.begin(), siblings.end(), this),
            siblings.end()
        );
    }

    parent = newParent;

    if (parent)
    {
        parent->children.push_back(this);

        localPosition.x = worldPosition.x - parent->worldPosition.x;
        localPosition.y = worldPosition.y - parent->worldPosition.y;

        localAngle = worldAngle - parent->worldAngle;
    }
}

void UICoreComponent::UpdateTransform()
{
    if (parent)
    {
        // ˆÊ’u
        worldPosition.x = parent->worldPosition.x + localPosition.x;
        worldPosition.y = parent->worldPosition.y + localPosition.y;

        // ‰ñ“]
        worldAngle = parent->worldAngle + localAngle;
    }

    for (auto* child : children)
    {
        child->UpdateTransform();
    }
}