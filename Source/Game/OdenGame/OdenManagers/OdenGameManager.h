#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

class CoreAudioSourceComponent;

// 全体の管理
// スコア管理
// ゲーム進行

class OdenGameManager :public Actor
{
public:
    enum class EFeverState
    {
        Charging,   // 溜め中
        Fever       // フィーバー中
    };

public:
    OdenGameManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // 総スコアを取得する
    float GetTotalScore()const { return totalScore; }

    // 残り時間を取得する
    float GetRemainingTime() const { return remainingTime; }

    // 設定時間
    float GetMaxTime() const { return maxTime; }

    // ゲームのステートをリセットする
    void Reset();

    // スコアを加算する
    void AddScore(float score);

    // 提出成功時の処理
    void OnSubmitSuccess();

    // フィーバー開始
    void StartFeverMode();

    // フィーバー終了
    void EndFever();

    // スコアを二倍にする
    float CalcScore(float baseScore) const;

    // 現在ゲージの値を取得する
    float GetFeverGauge() const
    {
        return feverGauge;
    }

    // ゲージの最大値を取得する
    float GetFeverGaugeMax() const
    {
        return feverGaugeMax;
    }

    // コンボ数を取得する
    int GetCombo() const
    {
        return combo;
    }

    // コンボをリセットする
    void ResetCombo()
    {
        combo = 0;
    }

    // タイムアップ
    bool IsTimeUp() const
    {
        return remainingTime <= 0.0f;
    }

    // ゲーム終了処理
    void EndGame();

    // ゲーム開始処理
    void StartGame();

    // 満足度加算
    void AddSatisfaction(const float value)
    {
        satisfaction = std::clamp(satisfaction + value, 0.0f, 100.0f);
    }

    // 提出ログを追加
    void AddSubmitLog(EOdenType type, float score);

    // フィーバーモードかどうか
    bool IsFeverMode()const
    {
        return feverState == EFeverState::Fever;
    }

    void SetBgmAudio(const std::shared_ptr<CoreAudioSourceComponent>& bgm) { bgmAudio = bgm; }

private:
    // コンボを加算する
    void AddCombo()
    {
        combo++;
    }
private:
    std::weak_ptr<CoreAudioSourceComponent> bgmAudio;
    std::shared_ptr<EasingRunner> easingBgm;

    //std::vector<OdenSubmitLog> submitLogs; 
    //std::array<int, static_cast<size_t>(EOdenType::Count)> ingredientCount{};
    float totalScore = 0.0f;
    int   combo = 0;

    float maxTime = 0.0f;      // 50秒
    float remainingTime = 50.0f;

    float satisfaction = 0.0f;  // 満足度

    bool isGameEnded = false; // ゲームが終わったかどうか
    bool isGameRunning = false; // ゲーム開始かどうか

    EFeverState feverState = EFeverState::Charging;

    float feverGauge = 0.0f;
    float feverGaugeMax = 100.0f;

    float feverTime = 8.0f;
    float feverRemainingTime = 0.0f;

    int feverTriggerCombo = 5;

    float bgmPitch = 1.0f;
    
};