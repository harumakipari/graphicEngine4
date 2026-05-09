#pragma once
#include "Core/Actor.h"

// タイトルステージモデルアクター
class TitleBookActor :public Actor
{
public:
    explicit TitleBookActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<SkeletalMeshComponent> stageModelComponent;

};

