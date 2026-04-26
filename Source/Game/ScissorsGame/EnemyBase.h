#pragma once
#include "EnemyBehavior.h"
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class ScissorsPlayer1;

class EnemyBase :public Character
{
public:
    enum class YarnState
    {
        Active,        // 通常
        Tied,          // 球止め（動けない）
        Dead
    };

    enum YarnSize
    {
        Small,
        Big,
    };

public:
    explicit EnemyBase(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // 敵のスコアを取得する関数
    EnemyScoreData GetScoreData() const { return scoreData; }

    // 死亡したかどうか
    bool IsDead() const { return isDead; }

    // 玉止めに必要な回数
    int GetNeedTiedCount()const { return (size == YarnSize::Big) ? 2 : 1; }

    // ダッシュ攻撃時に呼ぶ関数
    bool OnHitByDash();

    // 移動
    void Move(const DirectX::XMFLOAT3& dir, float deltaTime);

    // 向き
    void Face(const DirectX::XMFLOAT3& dir);

    // 振る舞いセット関数
    void SetBehavior(std::unique_ptr<EnemyBehavior> newBehavior);

    // サイズのセット関数
    void SetEnemySize(const YarnSize size);

    // 移動方向を取得する
    DirectX::XMFLOAT3 GetMoveDirection() const { return moveDirection; }

    // 移動方向を設定する
    void SetMoveDirection(const DirectX::XMFLOAT3 moveDir) { moveDirection = moveDir; }

    // 速度を取得する
    float GetSpeed() const { return speed; }

    // 速度を設定する
    void SetSpeed(const float speed) { this->speed = speed; }

    // 敵の出現開始位置を取得する
    DirectX::XMFLOAT3 GetStartPosition() const { return startPosition; }

    void SetStartPosition(const DirectX::XMFLOAT3 startPos) { this->startPosition = startPos; }

    // プレイヤーを取得する
    ScissorsPlayer1* GetPlayer();
private:
    // 死亡した時に呼ぶ関数
    void CallDeath(bool hitByDash);

    // コインを生成する
    void SpawnCoin(DirectX::XMFLOAT3 pos);

    // ヒットエフェクトを生成する
    void SpawnHitEffect(bool hitByDash);

    // 玉止めされている時の更新処理
    void UpdateTied(float deltaTime);

    // 玉止めをほどく
    void ReleasedTied();

    // 死亡中の更新処理
    void UpdateDead(float deltaTime);

    // 玉止め表示更新処理
    void UpdateTiedVisual();

public:
    // 死亡通知
    std::function<void()> onDeath;

    // 最後にヒットしたダッシュ区間
    int lastHitSegment = -1;
protected:
    DirectX::XMFLOAT3 startPosition = { 0.0f,0.0f,0.0f };   // 敵の出現の開始位置　波うちの時に基準とする
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加
    std::vector<std::shared_ptr<SkeletalMeshComponent>> tiedMeshes;// 玉止め用のモデル
    std::shared_ptr<RotationComponent> rotationComponent; // 回転のコンポーネント
    std::shared_ptr<SphereComponent> sphereCollisionComponent; // 当たり判定のコンポーネント

    // 移動のパラメータ
    DirectX::XMFLOAT3 moveDirection = { 1.0f, 0.0f, 0.0f }; // 線形移動の方向
    float speed = 2.0f; // 線形移動の速度

    float enemyRadius = 1.0f; // 敵の当たり判定
    int maxHp = 1;
    EnemyScoreData scoreData = { 100,0 }; // 倒したときのスコア

    int tieCount = 0;      // 何回止められたか
    float tieTimer = 0.0f; // 自力解除用

    YarnState state = YarnState::Active;
    YarnSize size = YarnSize::Small;
private:
    bool isDead = false;// 死亡したかどうか
    float deathTimer = 0.0f;

    bool createCoin = false; //  コインを生成する

    // ノックバックのデータ
    struct KnockbackData
    {
        DirectX::XMFLOAT3 startPos;
        DirectX::XMFLOAT3 targetPos;
        float height;
        float duration;
        float elapsedTime;
    };
    KnockbackData knockback;
    bool isKnockbackActive = false;

    float hitFlashTimer = 0.0f; // フラッシュタイマー
    float hitFlashDuration = 0.5f; // フラッシュ全体時間
    bool isDying = false;

    std::unique_ptr<EnemyBehavior> behavior; // 振る舞い

    std::string parentName = "EnemyBase";// RootComponentの名前

    std::shared_ptr<SphereComponent> redirectLeftCollisionComponent; // 反射判定の左コンポーネント

};
