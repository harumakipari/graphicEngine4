#pragma once
#include "EnemyScoreData.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"

class EnemyBase;

class BobbinActor :public Actor
{
public:
    enum class BobbinSize :uint8_t
    {
        Small,
        Medium,
        Big,
    };

private:
    enum class BobbinState:uint8_t
    {
        CoolDown, // クールダウン
        Charging, //溜め中
        Fired, // 発動した瞬間
        Executing,
    };
public:
    explicit BobbinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // ボビンのサイズを設定する
    void SetBobbinSize(BobbinSize bobbinSize);
private:
    // ボビンを使用する
    void UseBobbin();

    //　機能をリセットする
    void Reset();

    // 敵を玉止めする
    void ApplyToEnemies(DirectX::XMFLOAT3 center);

    // ボーナスボタンを生成する
    void SpawnBonusCoinBurst();
private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加
    std::shared_ptr<CoreAudioSourceComponent> chargeAudioComponent;   // ボビンのチャージ音のオーディオコンポーネント

    BobbinState bobbinState = BobbinState::Charging;

    float currentRadius = 0.0f;
    std::unordered_set<EnemyBase*> hitEnemies;
    float cooldownTimer = 0.0f;
    float chargeTimer = 0.0f;

    // 最後に当たったダッシュを記録する
    int lastUsedDashSerial = -1;

    // 調整
    float maxRadius = 6.0f; // 最大半径
    float cooldownInterval = 0.1f;// クールタイム
    float chargeTime = 3.5f; // 何秒でMaxになるか

};

