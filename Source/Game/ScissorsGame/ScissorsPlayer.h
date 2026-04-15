#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"


class ScissorsActor;

class ScissorsPlayer :public Character
{
public:
    explicit ScissorsPlayer(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ハサミの所持数を取得
    int GetScissorsCount() const { return scissorsCount; }
private:
    // ハサミを落とす
    void DropOne();

    // ハサミを拾う
    void PickUpNearest();

    // ハサミを引き寄せる
    void PullNearest();

    // 近くに落ちているハサミがあるか
    ScissorsActor* FindNearestDroppedScissors();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;

    int scissorsCount = 2; // ハサミの所持数
    std::vector<std::weak_ptr<ScissorsActor>> equippedScissors; // 手に持ってる
    std::vector<std::weak_ptr<ScissorsActor>> droppedScissors;  // 落ちてる

    float pickupRange = 1.0f; // ハサミを拾う範囲
};
