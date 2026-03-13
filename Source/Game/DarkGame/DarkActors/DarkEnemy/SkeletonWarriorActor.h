#pragma once
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"


class SkeletonWarriorActor :public Character
{
public:
    explicit SkeletonWarriorActor(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;

    std::vector<DirectX::XMFLOAT3> waypoints;
    int currentWaypoint = 0;

    float moveSpeed = 2.0f;
};




