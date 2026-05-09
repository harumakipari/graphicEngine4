#pragma once
#include "Core/Actor.h"

// タイトルステージモデルアクター
class TitleStageActor :public Actor
{
public:
    explicit TitleStageActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<SkeletalMeshComponent> stageModelComponent;

};

