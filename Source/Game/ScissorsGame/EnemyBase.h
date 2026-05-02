#pragma once
#include "EnemyAttack.h"
#include "EnemyBehavior.h"
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class ScissorsGameElasticMeshComponent;
class ScissorsPlayer1;

class EnemyBase :public Character
{
public:
    enum class YarnState
    {
        Active,        // 通常
        Tying,  // 玉止め途中　（サイズが大きい敵のみ、速度が遅くなる）
        Tied,          // 玉止め（動けない）
        Dead
    };

    enum YarnSize
    {
        Small,
        Big,
    };

public:
    explicit EnemyBase(const std::string& actorName) :Character(actorName)
    {
        std::random_device rd;
        rng.seed(rd());
    }

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 敵のスコアを取得する関数
    EnemyScoreData GetScoreData() const { return scoreData; }

    // 死亡したかどうか
    bool IsDead() const { return isDead; }

    // 玉止めに必要な回数
    int GetNeedTiedCount()const { return (yarnSize == YarnSize::Big) ? 2 : 1; }

    // ダッシュ攻撃時に呼ぶ関数
    bool OnHitByDash();

    // 強制的に玉止めにする
    void ForceTied();

    // 移動
    void Move(const DirectX::XMFLOAT3& dir, float deltaTime);

    // 向き
    void Face(const DirectX::XMFLOAT3& dir);

    // 振る舞いセット関数
    void SetBehavior(std::unique_ptr<EnemyBehavior> newBehavior);

    // 攻撃セット関数
    void SetAttack(std::unique_ptr<EnemyAttack> newAttack);

    // サイズのセット関数
    void SetEnemySize(const YarnSize size) { this->yarnSize = size; }

    // 移動方向を取得する
    DirectX::XMFLOAT3 GetMoveDirection() const { return moveDirection; }

    // 移動方向を設定する
    void SetMoveDirection(const DirectX::XMFLOAT3 moveDir) { moveDirection = moveDir; }

    // 速度を取得する
    float GetSpeed() const
    {
        if (state == YarnState::Tying)
        {// 半分玉止めされていたら
            return speed * 0.5f;
        }
        return speed;
    }

    // 速度を設定する
    void SetSpeed(const float speed) { this->speed = speed; }

    // 敵の出現開始位置を取得する
    DirectX::XMFLOAT3 GetStartPosition() const { return startPosition; }

    // 敵の出現開始位置を設定する
    void SetStartPosition(const DirectX::XMFLOAT3 startPos) { this->startPosition = startPos; }

    // プレイヤーを取得する
    ScissorsPlayer1* GetPlayer();

    // 状態を取得する関数
    YarnState GetState() const { return state; }

    // 玉止めをほどく
    void ReleasedTied();

    // ハサミを生成する
    void CreateScissorsVisual();

    // タイプを設定
    void SetEnemyType(const YarnEnemyType type) { enemyType = type; }

    // タイプとサイズから見た目を生成する
    void SetUpVisual();

    // 敵のサイズを変更する
    void ChangeSize(YarnSize newSize);

    // 助けるか銅貨を設定する
    void SetIsRescue(const bool rescue)
    {
        this->isRescuing = rescue;
    }
private:
    // 死亡した時に呼ぶ関数
    void CallDeath(bool hitByDash);

    // コインを生成する
    void SpawnCoin(DirectX::XMFLOAT3 pos);

    // ヒットエフェクトを生成する
    void SpawnHitEffect(bool hitByDash);

    // 玉止めされている時の更新処理
    void UpdateTied(float deltaTime);

    // 死亡中の更新処理
    void UpdateDead(float deltaTime);

    // 玉止め表示更新処理
    void UpdateTiedVisual();

    // ハサミの角度を変更する処理
    void UpdateScissors(float deltaTime);

public:
    // 死亡通知
    std::function<void()> onDeath;

    // 最後にヒットしたダッシュ区間
    int lastHitSegment = -1;

    // 予約用の敵（玉止めを外す時）
    EnemyBase* reservedBy = nullptr;

    bool isCutting = false; // 初回の切る
    float scissorsCutTimer = 0.0f;
    float rescueTimer = 0.0f;

    // 調整値
    const float prepareTimeInterval = 1.3f; //敵のハサミの切る準備時間
    const float cutTimeInterval = 0.2f; //敵のハサミの切るのにかかる時間

    // 乱数エンジン
    std::mt19937 rng;

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
    YarnSize yarnSize = YarnSize::Small;


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

    float selfRescueTimeInterval = 15.0f;// 自力脱出までかかる時間
    const float selfBigRescueTimeInterval = 8.0f;// 大きい敵が自力脱出までかかる時間
    const float selfSmallRescueTimeInterval = 15.0f;// 小さい敵が自力脱出までかかる時間

    float hitFlashTimer = 0.0f; // フラッシュタイマー
    float hitFlashDuration = 0.5f; // フラッシュ全体時間
    bool isDying = false;

    std::unique_ptr<EnemyBehavior> behavior; // 振る舞い
    std::unique_ptr<EnemyAttack> attack; // 攻撃

    std::string parentName = "EnemyBase";// RootComponentの名前

    std::shared_ptr<SphereComponent> redirectCollisionComponent; // 反射判定の左コンポーネント

    std::shared_ptr<SkeletalMeshComponent> scissorsFirstMeshComponent;// ハサミ描画用コンポーネント
    std::shared_ptr<SkeletalMeshComponent> scissorsSecondMeshComponent;// ハサミ描画用コンポーネント

    YarnEnemyType enemyType = YarnEnemyType::Static; // 敵のタイプ

    std::shared_ptr<ScissorsGameElasticMeshComponent> elasticMeshComponent; // 弾性描画用コンポーネント


    bool isRescuing = false; // 助けているかどうか
    float scissorsAnimTime = 0.0f; // ハサミの時間

    DirectX::XMFLOAT3 basePosition = { 0.0f,0.0f,0.0f };

    float shakeTimer = 0.0f; // 振動の時間
};
