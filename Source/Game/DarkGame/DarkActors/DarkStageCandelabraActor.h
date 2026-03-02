#pragma once
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStageCandelabraActor :public Actor
{
public:
    DarkStageCandelabraActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStageCandelabraActor() = default;
    void Initialize(const Transform& transform)override;
  
private:
    // êCë‰ÇÃÉÇÉfÉã
    std::shared_ptr<SkeletalMeshComponent> candelabraMeshComponent;

};