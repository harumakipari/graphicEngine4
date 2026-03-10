#pragma once
#include "Game/DarkGame/DarkActors/InteractableActor.h"

class DoorLeftActor : public InteractableActor
{
public:
    enum class DoorState :uint8_t
    {
        Closed,
        Opening,
        Open
    };

public:
    explicit DoorLeftActor(const std::string& actorName) :InteractableActor(actorName) {}

    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;

    // プレイヤーが押した時に呼ぶ 
    void Interact() override;

private:
    std::shared_ptr<SkeletalMeshComponent> meshComponent;
    bool isOpening = false;
    float openAngle = 0.0f;

    DoorState doorState = DoorState::Closed;
};

class DoorRightActor : public InteractableActor
{
public:
    enum class DoorState :uint8_t
    {
        Closed,
        Opening,
        Open
    };

public:
    explicit DoorRightActor(const std::string& actorName) :InteractableActor(actorName) {}

    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;

    // プレイヤーが押した時に呼ぶ 
    void Interact() override;

private:
    std::shared_ptr<SkeletalMeshComponent> meshComponent;
    bool isOpening = false;
    float openAngle = 0.0f;

    DoorState doorState = DoorState::Closed;
};