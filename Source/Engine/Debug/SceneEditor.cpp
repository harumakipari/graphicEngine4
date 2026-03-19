#include "pch.h"
#include "SceneEditor.h"
#include <imgui.h>
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneJson.h"
#include "Engine/Scene/SceneState.h"

void SceneEditor::Draw()
{
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Always);
    ImGui::Begin("SceneTransition");
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

    ImGui::Begin("Save Scene Preset");

    // --- •Û‘¶Œn ---
    auto* scene = Scene::GetCurrentScene();
    static SceneState savedState;

    if (ImGui::Button("Save Scene"))
    {
        savedState.Capture(scene);
        SaveSceneState("./Data/Saves/ScenePresets/scenePreset.json", savedState);
    }

    if (ImGui::Button("Load Scene"))
    {
        LoadSceneState("./Data/Saves/ScenePresets/scenePreset.json", savedState);
        savedState.Apply(scene);
    }

    ImGui::End();

}
