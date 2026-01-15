#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

// 全体の管理
// スコア管理
// ゲーム進行

class OdenGameManager :public Actor
{
public:
    struct OdenSubmitLog
    {
        EOdenType type; // 具材の種類
        int count = 1;             // 基本1だが拡張用
        float score = 0.0f;
                            // 後々Great　Goodとか
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
    void AddScore(float score)
    {
        //if (IsTimeUp()) return;
        totalScore += score;
        Logger::Log(U8("今の総スコア") + std::to_string(totalScore));
    }

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
    void EndGame()
    {
        // OrderManager を止める
    }

    // 満足度加算
    void AddSatisfaction(const float value)
    {
        satisfaction = std::clamp(satisfaction + value, 0.0f, 100.0f);
    }

    // 提出ログを追加
    void AddSubmitLog(EOdenType type, float score);

    // リザルト用
    const std::vector<OdenSubmitLog>& GetSubmitLogs() const
    {
        return submitLogs;
    }

    // 提出した食材の種類と数を取得する関数
    const std::unordered_map<EOdenType, int>& GetIngredientCount() const
    {
        return ingredientCount;
    }
private:
    float totalScore = 0.0f;
    int   combo = 0;

    float maxTime = 50.0f;      // 50秒
    float remainingTime = 50.0f;

    std::vector<OdenSubmitLog> submitLogs; // 時系列（伝票）
    std::unordered_map<EOdenType, int> ingredientCount; // 集計

    float satisfaction = 0.0f;  // 満足度
};