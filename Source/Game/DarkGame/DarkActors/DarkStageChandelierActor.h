#pragma once
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStageChandelierActor :public Actor
{
public:
    DarkStageChandelierActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageChandelierActor() = default;
    void Initialize(const Transform& transform)override;

private:
    std::shared_ptr<SkeletalMeshComponent> chandelierMeshComponent;
};