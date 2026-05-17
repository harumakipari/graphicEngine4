#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"
#include "NumberModelDisplay.h"
#include "Components/Audio/CoreAudioSourceComponent.h"


// リザルト本アクター
class ResultBookActor :public BookBaseActor
{
    enum class ResultPhase :uint8_t
    {
        Wait,

        ShowEnemyScore,

        ShowCombo,

        ShowHeart,
        AddHeart,

        ShowRedirect,
        AddRedirect,

        ShowGather,
        AddGather,

        ShowTimeBonus,
        AddTimeBonus,

        AddTotalScore,

        PreShowRanking,
        ShowRanking,

        Complete,
    };


public:
    explicit ResultBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // シーン遷移が完了したらスコア表示開始
    void StartShowEnemyScore();

protected:
    // コントローラー対応の本が開く処理
    void HandlePadInput() override;

private:
    // 矢印ボタンのUIを作成する
    void CreateButtonArrow();

    // 敵撃破スコア表示
    void ShowEnemyScore();

private:
    std::shared_ptr<CoreAudioSourceComponent> scoreCountUpAudioComponent;   // スコアカウントアップ音のオーディオコンポーネント

    std::shared_ptr<UIImageComponent> timeClearImage; // 目標タイムクリア時の画像
    std::shared_ptr<UIImageComponent> remainClearImage; // 目標タイムを達成できなかったときの画像

    DirectX::XMFLOAT3 scoreRelativePosition = { 0.0f,0.0f,0.f };

    NumberDisplay totalScoreDisplay;    // トータルスコア
    NumberDisplay comboDisplay; // コンボ数
    NumberDisplay heartDisplay;    // HPボーナス
    NumberDisplay redirectDisplay;  // 反射ボーナス
    NumberDisplay gatherDisplay;  // まとめボーナス
    NumberDisplay secondDisplay;  // 秒数
    NumberDisplay minuteDisplay;  // 分数

    NumberDisplay ranking1Display;  // ランキング
    NumberDisplay ranking2Display;  // ランキング
    NumberDisplay ranking3Display;  // ランキング
    NumberDisplay ranking4Display;  // ランキング
    NumberDisplay ranking5Display;  // ランキング


    std::shared_ptr<SkeletalMeshComponent> numberModel;

    // スコアカウントアップ用
    int displayedTotalScore = 0;    // 現在表示されているトータルスコア
    int addTargetScore = 0;
    int addStep = 50;
    float addInterval = 0.02f;
    float addTimer = 0.0f;
    int currentTotalScore = 0;  // 現在のトータルスコア
    int startScore = 0; // スコア加算開始時のスコア
    
    ResultPhase resultPhase = ResultPhase::Wait;    // 結果表示のフェーズ
    float phaseTimer = 0.0f;

    float preShowScoreInterval = 0.3f;   // スコアを出す前の待ち時間
    bool phaseInitialized = false; //Phase開始時1回だけ処理
};

