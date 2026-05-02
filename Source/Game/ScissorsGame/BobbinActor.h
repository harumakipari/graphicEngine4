#pragma once
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"

class EnemyBase;

class BobbinActor :public Actor
{
    enum class BobbinState:uint8_t
    {
        CoolDown, // クールダウン
        Charging, //溜め中
        Fired, // 発動した瞬間
    };

public:
    explicit BobbinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

private:
    // ボビンを使用する
    void UseBobbin();

    //　機能をリセットする
    void Reset();

    // 敵を玉止めする
    void ApplyToEnemies(DirectX::XMFLOAT3 center);
private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加

    BobbinState bobbinState = BobbinState::Charging;

    float currentRadius = 0.0f;
    std::unordered_set<EnemyBase*> hitEnemies;
    float cooldownTimer = 0.0f;
    float chargeTimer = 0.0f;

    // 調整
    float maxRadius = 6.0f; // 最大半径
    float cooldownInterval = 0.5f;// クールタイム
    float chargeTime = 5.0f; // 何秒でMaxになるか
};

