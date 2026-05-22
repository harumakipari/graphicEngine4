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

        PreHighScore,
        HighScore,
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

    // 目標タイムまでのタイマー表示を更新
    void UpdateTimerDigits(int totalSeconds);

    // 演出開始
    void MedalPlay();

    // タイマーのワッペンの演出開始
    void TimerPatchPlay();

    // 矢印ボタンUIを表示する
    void ShowButtonArrow();


private:
    std::unique_ptr<EasingRunner> easingRunner;  // メダル用のeasingComponent
    std::unique_ptr<EasingRunner> easingTimeRunner;  // タイマークリア用のeasingComponent

    std::shared_ptr<CoreAudioSourceComponent> scoreCountUpAudioComponent;   // スコアカウントアップ音のオーディオコンポーネント

    std::shared_ptr<SkeletalMeshComponent> medalSkeletalMeshComponent;  // メダル表示用

    std::shared_ptr<SkeletalMeshComponent> timerPatchSkeletalMeshComponent;  // タイマーワッペン表示用

    std::shared_ptr<SkeletalMeshComponent> resultCrownModel;

    std::shared_ptr<UIImageComponent> remainClearImage; // 目標タイムクリア時の画像

    std::shared_ptr<UIImageComponent> resultImage; // リザルト画像
    std::shared_ptr<UIImageComponent> selectImage; // セレクト画像


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

    std::vector<std::shared_ptr<UIImageComponent>> timerDigits; // 残り時間を表示
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

    DirectX::XMFLOAT2 timerBonusUiPos = { 590.0f,880.0f };  // タイムボーナスのUIの位置
    DirectX::XMFLOAT2 timerBonusUiSize = { 521.0f,151.0f }; // タイムボーナスのUIのサイズ

    // 配置
    float spacing = 30.0f;
    DirectX::XMFLOAT2 numberSize = { 30.0f,45.0f };
    DirectX::XMFLOAT2 minuteSpacing = { -72.0f,-14.0f };
    DirectX::XMFLOAT2 secondSpacing = { -44.0f,-16.0f };

    bool isNewRecord = false; //新記録かどうか

    // メダル演出用
    float startScale = 3.0f;
    float endScale = 1.15f;
    float currentScale = 1.0f;

    float medalValue = 0.0f;
    float interval = 0.5f; // メダルが移動する時間

    // タイマーワッペン用
    float timerValue = 0.0f; 
    float startTimerScale = 3.0f;
    float endTimerScale = 1.0f;
    float currentTimerScale = 1.0f;

    bool isCleared = false;// クリアしたかどうか

    DirectX::XMFLOAT4 numberColor = { 0.605f,0.408f,0.129f ,1.0f};
    DirectX::XMFLOAT4 playerColor = { 1.0f,0.216f,0.088f ,1.0f};

    bool isPlayTimerPatch = false;
};

