#pragma once
#include "Game/State/StateBase.h"

class ScissorsPlayer1;

class ScissorsPlayerStateBase : public State
{
public:
    ScissorsPlayerStateBase(ScissorsPlayer1* player);
    virtual ~ScissorsPlayerStateBase() = default;
    // ステートに入った時のメソッド
    virtual void Enter() = 0;

    // ステートで実行するメソッド
    virtual void Execute(float deltaTime) = 0;

    // ステージから出ていくときのメソッド
    virtual void Exit() = 0;

    virtual const char* GetName() const = 0;

protected:
    ScissorsPlayer1* player = nullptr;

};

// 待機ステートオブジェクト
class ScissorsPlayerIdleState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerIdleState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    virtual ~ScissorsPlayerIdleState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Idle"; }
};

// 移動ステートオブジェクト
class ScissorsPlayerRunningState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerRunningState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    ~ScissorsPlayerRunningState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Running"; }
};

// 攻撃ステートオブジェクト
class ScissorsPlayerAttackingState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerAttackingState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    ~ScissorsPlayerAttackingState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Attack"; }

private:
    float attackTimer = 0.0f;
    bool hitDone = false;
};

// 突進ステートオブジェクト
class ScissorsPlayerChargeDashState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerChargeDashState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    ~ScissorsPlayerChargeDashState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "ChargeDash"; }

private:
    float minDistance = 4.0f; // ダッシュの最小距離　
    float maxDistance = 15.0f; // ダッシュの最大距離 

};

// 突進ステートオブジェクト
class ScissorsPlayerDashState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerDashState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    ~ScissorsPlayerDashState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Dash"; }

private:
    float dashDuration = 0.2f;   // ダッシュにかかる時間
    float elapsedTime = 0.0f;    // 経過時間
    DirectX::XMFLOAT3 startPos;  // 開始位置
};

// スタンステートオブジェクト
class ScissorsPlayerStunState : public ScissorsPlayerStateBase
{
public:
    // コンストラクタ
    ScissorsPlayerStunState(ScissorsPlayer1* player) :ScissorsPlayerStateBase(player) {}
    // デストラクタ
    ~ScissorsPlayerStunState() = default;
    // ステートに入った時のメソッド
    void Enter() override;
    // ステートで実行するメソッド
    void Execute(float deltaTime) override;
    // ステートから出ていくときのメソッド
    void Exit() override;
    // ステート名を取得
    const char* GetName() const override { return "Stun"; }

private:
    float stunTimer = 0.0f; // スタンタイマー
    float stunDuration = 0.1f; // スタンしている秒数
};

