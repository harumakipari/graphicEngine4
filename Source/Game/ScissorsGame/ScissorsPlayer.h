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

    // ハサミが返ってきたときの処理
    void OnScissorsReturned(ScissorsActor* scissors);

private:
    // ハサミを落とす
    void DropOne();

    // ハサミを拾う
    void PickUpNearest();

    // ハサミを引き寄せる
    void PullNearest();

    // ハサミを投げる
    void ThrowScissors(float power);

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

    bool isCharging = false;
    float chargeTime = 0.0f;

    float maxChargeTime = 1.0f;     // 最大溜め
    float maxThrowDistance = 10.0f; // 最大距離

    float lastStickPower = 0.0f;// スティックの最終的な力　溜めの強さや投げるときの力に使用する
};
