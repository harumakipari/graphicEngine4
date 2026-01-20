#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"



// 　
// 　星のエフェクト表示
//
class StarParticleActor :public Actor
{
public:
    explicit StarParticleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // エフェクト再生開始
    void StartParticle();

private:
    float a = 0.f;
    std::shared_ptr<EasingRunner> easingRunner;
    std::shared_ptr<ParticleComponent> particleComp;
    XMFLOAT3 startPos;
    XMFLOAT3 scorePos;

    float time = 0.0f;
    float duration = 3.8f;   // 全体時間
    float angle = 0.0f;
    float radius = 0.6f;
};