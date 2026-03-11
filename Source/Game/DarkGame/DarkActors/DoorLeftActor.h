#pragma once
#include "Game/DarkGame/DarkActors/InteractableActor.h"



class DoorLeftActor : public Actor
{
public:
    enum class DoorState :uint8_t
    {
        Closed,
        Opening,
        Open
    };

public:
    explicit DoorLeftActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;

    // プレイヤーが押した時に呼ぶ 
    void Interact() ;

    void DrawImGuiDetails() override; 

private:
    std::shared_ptr<SkeletalMeshComponent> leftMeshComponent;
    bool isOpening = false;
    float openAngle = 0.0f;
    float openSpeed = 90.0f; // 1秒で90度
    DoorState doorState = DoorState::Closed;
};

class DoorRightActor : public Actor
{
public:
    enum class DoorState :uint8_t
    {
        Closed,
        Opening,
        Open
    };

public:
    explicit DoorRightActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform) override;
    void Update(float deltaTime) override;

    // プレイヤーが押した時に呼ぶ 
    void Interact() ;

private:
    std::shared_ptr<SkeletalMeshComponent> meshComponent;
    bool isOpening = false;
    float openAngle = 0.0f;
    float openSpeed = 90.0f; // 1秒で90度

    DoorState doorState = DoorState::Closed;
};

class DoorActor : public InteractableActor
{
public:
    explicit DoorActor(const std::string& actorName) :InteractableActor(actorName) {}

    void Initialize(const Transform& transform) override;
    void Update(float dt) override;

    void Interact() override;

private:

    std::shared_ptr<SceneComponent> root;

    std::shared_ptr<SceneComponent> leftHinge;
    std::shared_ptr<SceneComponent> rightHinge;

    std::shared_ptr<SkeletalMeshComponent> leftDoorMesh;
    std::shared_ptr<SkeletalMeshComponent> rightDoorMesh;

    float openAngle = 0.0f;
    float openSpeed = 60.0f;

    enum class DoorState
    {
        Closed,
        Opening,
        Open
    };

    DoorState doorState = DoorState::Closed;
};
