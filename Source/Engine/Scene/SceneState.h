#pragma once
#include "Scene.h"
#include "SceneSetting.h"

struct SceneState
{
    SceneLightSaveData lightSaveData;
    SceneShaderConstants shader;
    CascadedShadowMapConstants cascadeShadow;
    FogConstants fog;
    SSRConstantBuffer ssr;
    SSAOConstantBuffer ssao;
    BloomConstantBuffer bloom;


    void Capture(Scene* scene)
    {
        auto& s = scene->GetSceneSettings();

        lightSaveData = s.sceneLightSaveData;
        shader = s.sceneShaderConstants;
        fog = s.fogConstants;
        ssr = s.ssrConstantBuffer;
        ssao = s.ssaoConstantBuffer;
        bloom = s.bloomConstantBuffer;
    }

    void Apply(Scene* scene)
    {
        auto& s = scene->GetSceneSettings();

        // sceneConstants‚Í•’Ê‚Éã‘‚«
        s.sceneLightSaveData.sceneConstants = lightSaveData.sceneConstants;

        // sharedLights‚Í“Á•Êˆµ‚¢
        if (!lightSaveData.sharedLights.empty())
        {
            s.sceneLightSaveData.sharedLights = lightSaveData.sharedLights;
        }

        s.sceneLightSaveData = lightSaveData;
        s.sceneShaderConstants = shader;
        s.fogConstants = fog;
        s.ssrConstantBuffer = ssr;
        s.ssaoConstantBuffer = ssao;
        s.bloomConstantBuffer = bloom;
    }
};