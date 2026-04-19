#pragma once
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"
#include "UI/Widgets/Widget.h"

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

    // 攻撃時に呼ぶ処理
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

    //　ダッシュを使用時に呼ぶ関数　これを呼ぶとダッシュの残り回数が減る
    void UseDash();

    // ダッシュが失敗した時に呼ぶ関数　これを呼ぶとダッシュの残り回数が減らない
    void FailDash();

    // ダッシュを止める処理　これを呼ぶとダッシュが止まる
    void StopDash();

    // ポーズの時に呼ぶ関数　これを呼ぶと歩きのSEが止まる
    void OnPause();

private:
    // 入力から狙いの情報を取得する
    AimData GetAimData(const MoveIntent& intent, float deltaTime);

    // どの方向を向くか
    DirectX::XMFLOAT3 GetLookDirection() const;

    // ダッシュの回数を回復する関数　
    void RecoverDash(float deltaTime);

    // HPを表示するUIを更新する関数　
    void UpdateHpUI();

public:
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;
    DirectX::XMFLOAT3 targetPos = { 0.0f,0.0f,0.0f }; // ダッシュの移動先
    float hitStopTimer = 0.0f; // ヒットストップのタイマー　攻撃が当たったときに一定時間動きを止めるために使用する
    // ダッシュの狙いを表示する矢印のUIコンポーネント
    std::shared_ptr<UIImageComponent> dashAimArrowComponent;


    std::shared_ptr<CoreAudioSourceComponent> footstepAudioComponent;   // 歩行音のオーディオコンポーネント
    std::shared_ptr<CoreAudioSourceComponent> dashAudioComponent;   // ダッシュ音のオーディオコンポーネント
    std::shared_ptr<CoreAudioSourceComponent> chargeAudioComponent; // チャージ音のオーディオコンポーネント

    DirectX::XMFLOAT4 debugDashCollisionColor = { 1,1,1,0 }; // ダッシュ攻撃の当たり判定の色　通常は透明で、攻撃中は赤くするなどして使用する
    DirectX::XMFLOAT4 debugScissorsCollisionColor = { 1,1,1,0.5f }; // プレイヤーのハサミ攻撃当たり判定の色　通常は白色で、ダメージを受けたときに赤くするなどして使用する

private:
    std::shared_ptr<SphereComponent> dashAttackSphere; // ダッシュ攻撃の当たり判定用のSphereComponent
    std::shared_ptr<SphereComponent> scissorsAttackSphere; // ハサミ攻撃の当たり判定用のSphereComponent
    std::shared_ptr<SphereComponent> sphereComponent; // プレイヤーの当たり判定用のSphereComponent
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
    int maxDashCount = 50; // ダッシュの最大回数

    float dashRecoverTimer = 0.0f;
    float dashRecoverInterval = 10.0f; // ダッシュ回復のインターバル

    bool isCharging = false; // ダッシュを溜めているかどうか
    float chargeTime = 0.0f; // ダッシュの溜め時間
    float maxChargeTime = 1.0f;   // ダッシュの最大溜め時間　この時間以上溜めてもさらに強くならない

    // プレイヤーのHPを表示するUI　
    std::vector<std::shared_ptr<UIImageComponent>> hpUiComponents;

    //　デバック用
    DirectX::XMFLOAT4 debugPlayerCollisionColor = { 1,1,1,1 }; // プレイヤーの当たり判定の色　通常は白色で、ダメージを受けたときに赤くするなどして使用する

    std::unordered_set<YarnEnemyActor*> hitEnemies; // ハサミ攻撃で当たった敵を記録するためのセット　これに入っている敵にはハサミ攻撃のダメージを与えないようにする

    // 調整用のパラメータ　これらを調整してゲームバランスを取る
    float dashAttackRange = 0.8f; // ダッシュ攻撃の範囲　dashAttackSphereの半径と同じにする
    float scissorsAttackRange = 1.0f; // ハサミ攻撃の範囲　scissorsAttackSphereの半径と同じにする
    float playerRadius = 0.5f; // プレイヤーの当たり判定の半径　sphereComponentの半径と同じにする

    int scissorsDamage = 1;// ハサミ攻撃時に与えるダメージ
    int dashDamage = 1;// ダッシュ攻撃時に与えるダメージ


    float damageCooldown = 0.0f; // ダメージを受けた後の無敵時間のクールダウン



    float dashTime = 0.3f;
    float dashTimer = 0.0f;

    float dashSpeed = 0.0f;
    float minDashDistance = 3.0f;
    float maxDashDistance = 12.0f;

    DirectX::XMFLOAT3 dashDir = { 0,0,1 };


    float maxThrowDistance = 10.0f; // 最大距離

    float lastStickPower = 0.0f;// スティックの最終的な力　溜めの強さや投げるときの力に使用する
    float dashDistance;
    DirectX::XMFLOAT3 dashStartPos;

};
