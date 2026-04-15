#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"


class ScissorsStage :public Actor
{
public:
    explicit ScissorsStage(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<StaticMeshComponent> staticMeshComponent;
};
