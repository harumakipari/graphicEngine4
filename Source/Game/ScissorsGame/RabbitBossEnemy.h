#pragma once
#include "Core/Actor.h"
#include "EnemyBase.h"
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "UI/Widgets/Widget.h"


class RabbitBossEnemyActor :public EnemyBase
{
    enum class BossState:uint8_t
    {
        Normal,
        Stunned, // ボビンで止められている時
    };

public:
    explicit RabbitBossEnemyActor(const std::string& actorName) :EnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // スタン状態かどうか
    bool IsStunned() const { return bossState == BossState::Stunned; }

    // ダメージ処理
    float ComputeDamage(const BossDamageContext& damageContext);

    // 半透明の処理
    void SetRenderOpacity(float opacity);

    // 被ダメージ処理
    void TakeDamage(const int damage) { hp -= damage; }

private:
    // ランダムに大きい敵に変更する処理
    void EnlargeRandomEnemies(int count);

    // ダメージが入る場所を生成する
    void CreteDamageZone(const DirectX::XMFLOAT3& pos);

    // ランダムな場所から出現する処理
    void SpawnRandomPoint();

private:
    BossState bossState = BossState::Normal;
    std::vector<DirectX::XMFLOAT3> spawnPoints; // 出現位置

    float attackTimer = 0.0f;

    const float attackTimeInterval = 5.0f; // 攻撃の間隔
    float spawnAttackRange = 3.0f;// 出現時の攻撃範囲


    std::shared_ptr<UIImageComponent> gaugeFrameBackComponent;  // ボスHPゲージのスプライト描画
    std::shared_ptr<UIGaugeComponent> gaugeUi; // ボスHPのゲージUI
    DirectX::XMFLOAT2 gaugeUiOffset={-184.0f,-366.0f}; // ゲージのUIオフセット値
    DirectX::XMFLOAT2 gaugeFrameOffset = { 8.0f,0.0f }; // ゲージフレームのオフセット値


};

