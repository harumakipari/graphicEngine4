#pragma once
#include "Core/Actor.h"

class DarkStageBrazierActor :public Actor
{
public:
    DarkStageBrazierActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageBrazierActor() = default;
    void Initialize(const Transform& transform)override;

private:
    // ‰Î”«‚Ìƒ‚ƒfƒ‹
    std::shared_ptr<SkeletalMeshComponent> brazierMeshComponent;
};