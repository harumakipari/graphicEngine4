#pragma once
#include "Core/Actor.h"


class YarnWallActor :public Actor
{
public:
    explicit YarnWallActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent; // 描画コンポーネント
    std::shared_ptr<BoxComponent> redirectCollisionComponent;// 反射コンポーネント
};

