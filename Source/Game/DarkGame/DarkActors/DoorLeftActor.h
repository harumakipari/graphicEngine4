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

    // ƒvƒŒƒCƒ„[‚ª‰Ÿ‚µ‚½‚ÉŒÄ‚Ô 
    void Interact(Player* player) override;

private:
    std::shared_ptr<SkeletalMeshComponent> meshComponent;
    bool isOpening = false;
    float openAngle = 0.0f;

    DoorState doorState = DoorState::Closed;
};