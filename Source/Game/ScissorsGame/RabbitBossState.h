#pragma once
#include <random>

#include "Game/State/StateBase.h"

class RabbitBossEnemyActor;

class RabbitBossStateBase : public State
{
public:
    RabbitBossStateBase(RabbitBossEnemyActor* enemy);
    virtual ~RabbitBossStateBase() = default;
    // ステートに入った時のメソッド
    virtual void Enter() = 0;

    // ステートで実行するメソッド
    virtual void Execute(float deltaTime) = 0;

    // ステージから出ていくときのメソッド
    virtual void Exit() = 0;

    virtual const char* GetName() const = 0;

protected:
    RabbitBossEnemyActor* enemy = nullptr;

};

// 待機ステートオブジェクト
class RabbitBossIdleState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossIdleState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossIdleState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Idle"; }

private:
    float attackTimer = 0.0f;
    float attackTimerInterval = 4.0f; // 攻撃までのインターバル
};


// 攻撃選択オブジェクト
class RabbitBossAttackSelectState : public RabbitBossStateBase
{
    enum class BossAttackType :uint8_t
    {
        Warp,
        Buff
    };
public:
    // コンストラクタ
    RabbitBossAttackSelectState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossAttackSelectState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "AttackSelect"; }

private:
    // 攻撃を選ぶバッグを生成する
    void RefillAttackBag();

    // 攻撃タイプを取り出す
    BossAttackType PopAttack();

private:
    std::vector<BossAttackType> attackBag;  // 攻撃タイプのバッグ
    std::mt19937 rng{ std::random_device{}() };

};

// ワーププレビューオブジェクト
class RabbitBossAttackWarpPreviewState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossAttackWarpPreviewState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossAttackWarpPreviewState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "WarpPreview"; }

private:
};

// ワープオブジェクト
class RabbitBossAttackWarpState : public RabbitBossStateBase
{
    enum class WarpPhase :uint8_t
    {
        Dive,     // 潜る
        Chase,    // 追尾
        ChaseEnd, // 追尾終了
        Warning,  // 警告
        Emerge    // 出現
    };

public:
    // コンストラクタ
    RabbitBossAttackWarpState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossAttackWarpState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Warp"; }

private:
    WarpPhase phase = WarpPhase::Dive;
    float timer = 0.0f;

    const float diveTime = 0.5f;
    const float chaseTime = 2.5f;   // 追尾時間
    const float chaseEndTime = 0.5f;   // 追尾終了演出時間
    const float warningTime = 1.0f; // 予告時間

};

// バフプレビューオブジェクト
class RabbitBossAttackBuffPreviewState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossAttackBuffPreviewState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossAttackBuffPreviewState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "BuffPreview"; }

private:
};

// バフオブジェクト
class RabbitBossAttackBuffState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossAttackBuffState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossAttackBuffState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Buff"; }

private:
    int enemyBuffCount = 1; // ステージ上のどれくらいの数の敵を強化させるか
};

// スタンオブジェクト
class RabbitBossStunState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossStunState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossStunState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Stun"; }

private:
    float stunTimer = 0.0f;
    float stunTimerInterval = 5.0f; // 何秒間スタンさせるか
};

// 死亡オブジェクト
class RabbitBossDeathState : public RabbitBossStateBase
{
public:
    // コンストラクタ
    RabbitBossDeathState(RabbitBossEnemyActor* enemy) :RabbitBossStateBase(enemy) {}
    // デストラクタ
    virtual ~RabbitBossDeathState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Death"; }

private:
    float elapsedTime = 0.0f;   // 経過時間

};