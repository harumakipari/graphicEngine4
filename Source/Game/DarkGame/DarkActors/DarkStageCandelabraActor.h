#pragma once
#include "DarkStageAsset.h"
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStageCandelabraActor :public Actor
{
    struct FlameSettings
    {
        float baseEmission = 6.0f;

        float flickerSpeed1 = 7.3f;
        float flickerSpeed2 = 13.7f;
        float flickerAmp1 = 0.05f;
        float flickerAmp2 = 0.02f;

        float baseScale = 0.006f;
        float scaleAmp = 0.0002f;
        float heightMultiplier = 3.1f;

        float posAmpX = 0.001f;
        float posAmpY = 0.00f;
        float posAmpZ = 0.001f;

        float colorBaseG = 0.3f;
        float colorAmpG = 0.15f;
    };
    FlameSettings flameSettings = {};
public:
    DarkStageCandelabraActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageCandelabraActor() = default;
    void Initialize(const Transform& transform)override;
    void Update(float deltaTime) override;
    void SetModel(const std::shared_ptr<StageAsset>& stageAsset);
    void DrawImGuiDetails() override;
private:
    // C‘ä‚Ìƒ‚ƒfƒ‹
    std::shared_ptr<SkeletalMeshComponent> candelabraMeshComponent;
    std::vector<SkeletalMeshComponent*> flameComponents; // ‰Š‚Ìƒ‚ƒfƒ‹
    std::vector<DirectX::XMFLOAT3> flameBasePositions;// ‰Š‚Ì‰ŠúˆÊ’u
};