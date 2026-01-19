#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

// 全体の管理
// スコア管理
// ゲーム進行

class OdenGameManager :public Actor
{
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

    // コンボを加算する
    void AddCombo()
    {
        combo++;
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


    // 満足度加算
    void AddSatisfaction(const float value)
    {
        satisfaction = std::clamp(satisfaction + value, 0.0f, 100.0f);
    }

    // 提出ログを追加
    void AddSubmitLog(EOdenType type, float score);


private:
    //std::vector<OdenSubmitLog> submitLogs; 
    //std::array<int, static_cast<size_t>(EOdenType::Count)> ingredientCount{};
    float totalScore = 0.0f;
    int   combo = 0;

    float maxTime = 0.0f;      // 50秒
    float remainingTime = 50.0f;


    float satisfaction = 0.0f;  // 満足度

    bool isGameEnded = false; // ゲームが終わったかどうか
};