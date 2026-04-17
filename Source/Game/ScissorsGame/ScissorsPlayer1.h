#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class YarnEnemyActor;

class ScissorsPlayer1 :public Character
{
    enum class State :uint8_t
    {
        Idle,
        Walking,
        ChargingDash,
        Dashing,
        Attacking
    };

    struct AimData
    {
        DirectX::XMFLOAT3 dir;
        float power;
        bool isValid;
    };

public:
    explicit ScissorsPlayer1(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ダメージを受ける処理
    void TakeDamage(int damage);

    void Attack();

private:
    // 入力から狙いの情報を取得する
    AimData GetAimData(const MoveIntent& intent, float deltaTime);

    // ダッシュを開始する処理
    void StartDush(const MoveIntent& intent);


private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;

    float damageCooldown = 0.0f; // ダメージを受けた後の無敵時間のクールダウン
    float pickupRange = 1.0f; // ハサミを拾う範囲

    State state = State::Walking; // プレイヤーの状態

    float dashChargeTime = 0.0f;
    float maxDashChargeTime = 1.0f;

    float dashTime = 0.3f;
    float dashTimer = 0.0f;

    float dashSpeed = 0.0f;
    float minDashDistance = 3.0f;
    float maxDashDistance = 12.0f;

    DirectX::XMFLOAT3 dashDir = { 0,0,1 };

    int dashCount = 3;
    int maxDashCount = 3;
    float dashRecoverTimer = 0.0f;
    float dashRecoverInterval = 1.0f;


    bool isCharging = false;
    float chargeTime = 0.0f;

    float maxChargeTime = 1.0f;     // 最大溜め
    float maxThrowDistance = 10.0f; // 最大距離

    float lastStickPower = 0.0f;// スティックの最終的な力　溜めの強さや投げるときの力に使用する
    float hitStopTimer = 0.0f;
    float dashDistance;
    DirectX::XMFLOAT3 dashStartPos;
    std::unordered_set<YarnEnemyActor*> hitEnemies; // すでに当たった敵

};
