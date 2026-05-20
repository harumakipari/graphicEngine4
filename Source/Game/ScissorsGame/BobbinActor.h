#pragma once
#include "EnemyScoreData.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

class EnemyBase;

class BobbinActor :public Actor
{
public:
    enum class BobbinSize :uint8_t
    {
        Small,
        Medium,
        Big,
        BossBobbin, // ボス戦で出てくるボビン
    };


private:
    enum class BobbinState:uint8_t
    {
        CoolDown, // クールダウン
        Charging, //溜め中
        ChargeEnd, // 溜め終わり
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

    // 初期状態でボビンをチャージする
    void SetBobbinStateCharge();

    // ボビンの見た目を非表示にする
    void HideBobbinVisual();
private:
    // ボビンを使用する
    void UseBobbin();

    //　機能をリセットする
    void Reset();

    // 敵を玉止めする
    void ApplyToEnemies(DirectX::XMFLOAT3 center);

    // ボーナスボタンを生成する
    void SpawnBonusCoinBurst();

    // ボビンに矢印を出す
    void UpdateShowArrow(float deltaTime);
private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加
    std::shared_ptr<CoreAudioSourceComponent> chargeAudioComponent;   // ボビンのチャージ音のオーディオコンポーネント
    std::shared_ptr<SkeletalMeshComponent> bobbinApplyRangeMeshComponent;// 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> bobbinStringMeshComponent;// 描画用コンポーネントを追加

    std::shared_ptr<UIImageComponent> arrowComponent;   // 矢印コンポーネント
    std::shared_ptr<UIImageComponent> tutorialComponent;   // チュートリアルイメージコンポーネント
    std::shared_ptr<UIImageComponent> tutorialChargeComponent;   // チュートリアルイメージコンポーネント
    std::shared_ptr<UIImageComponent> tutorialFirstComponent;   // チュートリアルイメージコンポーネント

    BobbinState bobbinState = BobbinState::Charging;

    float currentRadius = 0.0f;
    std::unordered_set<EnemyBase*> hitEnemies;
    float cooldownTimer = 0.0f;
    float chargeTimer = 0.0f;

    // 最後に当たったダッシュを記録する
    int lastUsedDashSerial = -1;

    float applyRangeMaxScale = 1.0f;// 床の広がるスケール

    float elapsedTime = 0.0f;   // 経過時間
    float tutorialElapsedTime = 0.0f; 
    float tutorialChargeElapsedTime = 0.0f; 
    XMFLOAT2 arrowOffsetPos = { 0.0f,50.0f };
    int useCount = 0; // 何回糸巻を使用したか

    // 調整
    float maxRadius = 6.0f; // 最大半径
    float cooldownInterval = 0.1f;// クールタイム
    float chargeTime = 3.5f; // 何秒でMaxになるか
    XMFLOAT2 tutorialPos = { 270.0f,130.0f };    // チュートリアル用の説明文の位置
    XMFLOAT2 tutorialSize = { 600.0f,250.0f }; // チュートリアル用の説明文のサイズ

    bool gameStart = false;
};

