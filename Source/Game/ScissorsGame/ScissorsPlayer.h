#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"


class ScissorsPlayer :public Character
{
public:
    explicit ScissorsPlayer(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;
};
