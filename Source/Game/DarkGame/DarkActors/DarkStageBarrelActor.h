#pragma once
#include "Core/Actor.h"

class DarkStageBarrelActor :public Actor
{
public:
    DarkStageBarrelActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageBarrelActor() = default;
    void Initialize(const Transform& transform)override;

private:
    // ’M‚Ìƒ‚ƒfƒ‹
    std::shared_ptr<SkeletalMeshComponent> barrelMeshComponent;
};