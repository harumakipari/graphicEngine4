#pragma once
#include "Core/Actor.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "UI/Widgets/Widget.h"



// 　
// 　星のエフェクト表示
//
class StarParticleActor :public Actor
{
public:
    enum class StarPhase
    {
        Orbit,      // 周りを回る
        Merge,      // 合流
        Attract     // スコアへ
    };

    struct TrailStar
    {
        std::shared_ptr<UIImageComponent> sprite;
        float life;
    };

public:

    explicit StarParticleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Finalize() override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // エフェクト再生開始
    void StartParticle();

private:
    void SpawnTrailStar(const XMFLOAT3& worldPos);

private:
    std::shared_ptr<EasingRunner> easingRunner;
    std::shared_ptr<ParticleComponent> particleComp;
    static constexpr int StarCount = 3;
    std::array<std::shared_ptr<UIImageComponent>, StarCount> starTextures;
    std::array<float, StarCount> starAngles;    XMFLOAT3 startPos;

    XMFLOAT3 scorePos;

    float time = 0.0f;
    float attractDuration = 1.2f;   // スコアへ移動の時間
    float mergeDuration = 0.6f; // 合流時間
    float orbitDuration = 0.6f; // 回転の時間
    float angle = 0.0f;
    float radius = 1.5f;
    StarPhase phase = StarPhase::Orbit;
    XMFLOAT3 prevTrailPos = { 0.0f,0.0f,0.0f };
    std::vector<TrailStar> trailStars;
};