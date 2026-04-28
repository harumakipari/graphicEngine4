#pragma once
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"

class BobbinActor :public Actor
{
public:
    explicit BobbinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加
};

