#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class YarnEnemyActor;

class ScissorsPlayer1 :public Character
{
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

    void DrawImGuiDetails() override;

    // ダメージを受ける処理
    void TakeDamage(int damage);

    // 攻撃処理
    void DoAttackHit();

    // 移動方向を取得する関数
    DirectX::XMFLOAT3 GetMoveDirection() const { return moveDir; }

    // ダッシュ溜めトリガーが離されたかどうかを取得する関数　これがtrueのときにダッシュを開始する 
    bool IsDashTriggered() const { return triggerDash; }

    // ダッシュ溜めトリガーが引かれたかどうかを取得する関数
    bool IsChargeDashTriggered() const { return triggerChargeDash; }

    // 攻撃トリガーが引かれたかどうかを取得する関数
    bool IsAttackTriggered() const { return attackTrigger; }

    // 狙いの情報を取得する関数　これでダッシュの方向や溜めの強さを決める
    AimData GetAimData() const { return aimData; }

    // ダッシュ可能かどうかを取得する関数
    bool CanDash() const { return dashCount > 0; }

    //　ダッシュを使用する関数　これを呼ぶとダッシュの残り回数が減る
    void UseDash();

    // ダッシュが失敗した時に呼ぶ関数　これを呼ぶとダッシュの残り回数が減らない
    void FailDash();

    // ダッシュを止める処理　これを呼ぶとダッシュが止まる
    void StopDash();
private:
    // 入力から狙いの情報を取得する
    AimData GetAimData(const MoveIntent& intent, float deltaTime);

    // どの方向を向くか
    DirectX::XMFLOAT3 GetLookDirection() const;

    // ダッシュの回数を回復する関数　
    void RecoverDash(float deltaTime);
    
public:
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;
    DirectX::XMFLOAT3 targetPos = { 0.0f,0.0f,0.0f }; // ダッシュの移動先
    float hitStopTimer = 0.0f; // ヒットストップのタイマー　攻撃が当たったときに一定時間動きを止めるために使用する

private:
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    DirectX::XMFLOAT3 moveDir = { 0,0,0 }; // 移動方向
    bool triggerDash = false; // ダッシュトリガー
    bool triggerChargeDash = false; // ダッシュ溜めを検知するトリガー
    bool attackTrigger = false; // 攻撃トリガー
    AimData aimData; // 狙いの情報

    bool usingStick = false; // スティックを使用しているかどうか
    bool stickReleased = false; // スティックが離されたかどうか
    bool useGamePad = false; // ゲームパッドを使用しているかどうか
    bool preUsingStick = false; // 前フレームでスティックを使用していたかどうか

    int dashCount = 3; // ダッシュの残り回数
    int maxDashCount = 3; // ダッシュの最大回数



    float dashRecoverTimer = 0.0f;
    float dashRecoverInterval = 10.0f; // ダッシュ回復のインターバル


    float damageCooldown = 0.0f; // ダメージを受けた後の無敵時間のクールダウン
    float pickupRange = 1.0f; // ハサミを拾う範囲


    float dashTime = 0.3f;
    float dashTimer = 0.0f;

    float dashSpeed = 0.0f;
    float minDashDistance = 3.0f;
    float maxDashDistance = 12.0f;

    DirectX::XMFLOAT3 dashDir = { 0,0,1 };



    bool isCharging = false;
    float chargeTime = 0.0f;

    float maxChargeTime = 1.0f;     // 最大溜め
    float maxThrowDistance = 10.0f; // 最大距離

    float lastStickPower = 0.0f;// スティックの最終的な力　溜めの強さや投げるときの力に使用する
    float dashDistance;
    DirectX::XMFLOAT3 dashStartPos;
    std::unordered_set<YarnEnemyActor*> hitEnemies; // すでに当たった敵

};
