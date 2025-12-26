#include "pch.h"
#include "SceneEditor.h"
#include <imgui.h>
#include "Engine/Scene/Scene.h"

void SceneEditor::Draw()
{
    if (!ImGui::Begin("Scene Transition")) return;

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
