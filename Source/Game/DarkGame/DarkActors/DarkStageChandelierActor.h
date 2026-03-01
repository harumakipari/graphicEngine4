#pragma once
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStageChandelierActor :public Actor
{
public:
    DarkStageChandelierActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageChandelierActor() = default;
    void Initialize(const Transform& transform)override;
    void Update(float deltaTime) override;
private:
    // シャンデリアのモデル
    std::shared_ptr<SkeletalMeshComponent> chandelierMeshComponent;

    float swingTime = 0.0f;
    float swingSpeed = 1.5f;     // 揺れる速さ
    float swingAngle = 10.0f;    // 最大角度（度）
};