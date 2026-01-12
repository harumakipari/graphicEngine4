#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

// 全体の管理
// スコア管理
// 次のおでん補充
// ビート管理
// ゲーム進行

class OdenGameManager:public Actor
{
public:
    OdenGameManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

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
private:
    float totalScore = 0.0f;
    int   combo = 0;

    float maxTime = 50.0f;      // 50秒
    float remainingTime = 50.0f;
};