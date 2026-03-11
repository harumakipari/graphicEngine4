#include "pch.h"
#include "SceneEditor.h"
#include <imgui.h>
#include "Engine/Scene/Scene.h"

void SceneEditor::Draw()
{
    ImGui::Begin(U8("ƒV[ƒ“‘JˆÚ"));

    ImGui::Text("Scenes");
    ImGui::Separator();

    for (const auto& sceneName : Scene::GetRegisteredSceneNames())
    {
        if (ImGui::Button(sceneName.c_str(), ImVec2(-1, 0)))
        {
            Scene::_transition(sceneName, {});
        }
    }
    ImGui::End();
}
