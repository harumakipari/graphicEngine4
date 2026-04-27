#pragma once
#include "Core/Actor.h"

// ”ò‚Î‚·j
class NeedleActor :public Actor
{
public:
    explicit NeedleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ”ò‚Î‚·•ûŒü
    void SetDirection(const DirectX::XMFLOAT3& dir) { this->velocity = dir; }

private:
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };
    float speed = 10.0f;

    std::shared_ptr<SkeletalMeshComponent> mesh;
    std::shared_ptr<SphereComponent> collision;
};

