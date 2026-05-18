#pragma once
#include "Core/Actor.h"
#include "EnemyBase.h"
#include "EnemyScoreData.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Controller/ControllerComponent.h"
#include "UI/Widgets/Widget.h"


class RabbitBossEnemyActor :public EnemyBase
{
    enum class DropType :uint8_t
    {
        Bomb,
        Heart,
    };

public:
    explicit RabbitBossEnemyActor(const std::string& actorName) :EnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // スタン状態かどうか
    bool IsStunned();

    // 
    bool IsTied()  override
    {
        return IsStunned();
    }

    // ダメージ処理
    float ComputeDamage(const BossDamageContext& damageContext);

    // 半透明の処理
    void SetRenderOpacity(float opacity);

    // 被ダメージ処理
    void TakeDamage(const int damage);

    // ボビンによって玉止めされた時
    void OnTied() override;

    // ランダムに大きい敵に変更する処理
    void EnlargeRandomEnemies(int count);

    // ボス出現時にプレイヤーを押し出す
    void PushPlayerOut();

    // 沈み開始する処理
    void StartDive()
    {
        isDiving = true;
        baseY = GetPosition().y;
    }

    // 出現を開始する処理
    void StartEmerge()
    {
        isEmerging = true;
        baseY = GetPosition().y;
    }

    // 沈みが終わる処理
    bool IsFinishedDive()
    {
        return !isDiving;
    }

    // 出現が終わる処理
    bool IsFinishedEmerge()
    {
        return !isEmerging;
    }

    // 着地ダメージを適用
    void ApplyLandingDamage()
    {
        CreteDamageZone();
        //PushPlayerOut();
    }

    // ボスが死亡したら呼ぶ処理  一フレームのみ
    void StartDeathPerform();

    // 死亡演出が終了した時に呼ぶ処理
    void EndDeathPerform(bool playerDeath);

    // 周囲の敵を非表示
    void HideNearByRadius(float radius);

    //　勝利時に呼ぶ更新処理
    void UpdateWin(float deltaTime) {}

    // 出現している全ての敵を玉止めする関数
    void ApplyTiedAllEnemy();

    // ボスの周りのモデルを非表示にする処理
    void HideAroundModel();

    // 出現攻撃範囲を取得する
    float GetAttackRange() { return spawnAttackRange; }

    // 出現範囲のモデルスケールを取得する
    float GetSpawnScale() { return spawnScaleMax; }

    // 死亡演出のポストエフェクトの半径を設定する
    void SetDeathRadius(float r) { this->deathRadius = r; }

    // 死亡演出のポストエフェクトの半径を取得する
    float GetDeathRadius() { return deathRadius; }

private:
    // ダメージが入る場所を生成する
    void CreteDamageZone();

    // スタン状態に入る
    void EnterStun();

    // 爆弾を生成する
    void SpawnButtonBombs();

    // Ｙ座標を下げる処理
    void ApplyDiveOffset();

    // 落とすアイテムを満たす処理
    void RefillDropBag();

    // アイテム取り出し処理
    DropType PopDrop();

public:
    std::shared_ptr<BoxComponent> collisionBoxComponent; // ボスの当たり判定
    std::shared_ptr<SkeletalMeshComponent> stunModel; // スタン中に表示するモデル
    std::shared_ptr<SkeletalMeshComponent> bossSpawnMarkModel; // ボスの出現の場所モデル
    std::shared_ptr<SkeletalMeshComponent> bossChaseMarkModel; // ボスの追尾の場所モデル
    std::shared_ptr<CoreAudioSourceComponent> bossStunAudioComponent;// ボスの混乱音コンポーネント
    std::shared_ptr<CoreAudioSourceComponent> bossPreBuffAudioComponent;// ボスの敵強化音コンポーネント

    DirectX::XMFLOAT3 stunModelInitAngle={0.0f,0.0f,0.0f};  // 初期のスタンモデルの角度

    // 再スタン防止タイマー
    float stunCooldownTimer = 0.0f;
    // 再スタン防止時間
    float stunCooldownDuration = 3.0f;

    // Win時に地面の下にいるかを確認する
    bool isUnderGround = false; // 地面に潜る攻撃を開始しているかどうか

private:
    std::vector<DirectX::XMFLOAT3> spawnPoints; // 出現位置

    float attackTimer = 0.0f;

    const float attackTimeInterval = 5.0f; // 攻撃の間隔
    float spawnAttackRange = 4.5f;// 出現時の攻撃範囲

    float spawnAttackEnemyRange = 4.0f;// 出現時の攻撃範囲


    std::shared_ptr<UIImageComponent> gaugeFrameBackComponent;  // ボスHPゲージのスプライト描画
    std::shared_ptr<UIGaugeComponent> gaugeUi; // ボスHPのゲージUI
    DirectX::XMFLOAT2 gaugeUiOffset = { 0.0f,0.0f }; // ゲージのUIオフセット値
    DirectX::XMFLOAT2 gaugeFrameOffset = { 6.0f,0.0f }; // ゲージフレームのオフセット値
    DirectX::XMFLOAT2 gaugeUiPos = { 540.0f,6.0f };   // ゲージのposition
    //DirectX::XMFLOAT2 gaugeUiPos = { 506.0f,34.0f };   // ゲージのposition

    float diveOffsetY = 0.0f;
    bool isDiving = false;
    bool isEmerging = false;

    const float maxDiveDepth = -7.0f;
    const float diveSpeed = 5.0f;
    const float diveEmergeSpeed = 10.0f; //出現のスピード
    float baseY = 0.0f; // 開始位置

    bool spawnBomb = false; // 爆弾を発生させるかどうか

    std::mt19937 rng{ std::random_device{}() };

    std::vector<DropType> dropBag;  // アイテムを落とす中身のバッグ

    bool endPerform = false;

    float spawnScaleMax = 1.65f; // 出現マークの大きさ

    CoreColor green = { 0.886f,1.0f,0.098f,1.0f };
    CoreColor orange = { 1.0f,0.5f,0.0f,1.0f };
    CoreColor red = { 1.0f,0.1f,0.1f,1.0f };

    float deathRadius = 10.0f; // 死亡時のポストエフェクトに使う半径
};

