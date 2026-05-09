#pragma once
#include "Core/Actor.h"

// トレイルモデルアクター
class TrailModelActor :public Actor
{
public:
    explicit TrailModelActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void SetDirection(DirectX::XMFLOAT3 dir);

private:
    std::shared_ptr<SkeletalMeshComponent> trailModelMeshComponent;
    DirectX::XMFLOAT3 eularDegree={0.0f,0.0f,0.0f};

};

