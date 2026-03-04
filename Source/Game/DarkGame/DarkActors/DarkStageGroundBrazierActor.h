#pragma once
#include "Core/Actor.h"

class DarkStageGroundBrazierActor :public Actor
{
public:
    DarkStageGroundBrazierActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageGroundBrazierActor() = default;
    void Initialize(const Transform& transform)override;

private:
    // ’n–Ê‚Ì‰Î”«‚Ìƒ‚ƒfƒ‹
    std::shared_ptr<SkeletalMeshComponent> brazierMeshComponent;
};