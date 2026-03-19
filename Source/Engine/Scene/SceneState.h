#pragma once
#include "Scene.h"
#include "SceneSetting.h"

struct SceneState
{
    SceneLightConstants light;
    SceneShaderConstants shader;
    FogConstants fog;
    SSRConstantBuffer ssr;
    SSAOConstantBuffer ssao;
    BloomConstantBuffer bloom;

    void Capture(Scene* scene)
    {
        auto& s = scene->GetSceneSettings();

        light = s.sceneLightConstants;
        shader = s.sceneShaderConstants;
        fog = s.fogConstants;
        ssr = s.ssrConstantBuffer;
        ssao = s.ssaoConstantBuffer;
        bloom = s.bloomConstantBuffer;
    }

    void Apply(Scene* scene)
    {
        auto& s = scene->GetSceneSettings();

        s.sceneLightConstants = light;
        s.sceneShaderConstants = shader;
        s.fogConstants = fog;
        s.ssrConstantBuffer = ssr;
        s.ssaoConstantBuffer = ssao;
        s.bloomConstantBuffer = bloom;
    }
};