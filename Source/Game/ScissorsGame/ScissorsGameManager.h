#pragma once

#include "Core/Actor.h"
// 全体の管理
// スコア管理
// ゲーム進行

class ScissorsGameManager :public Actor
{
public:
    ScissorsGameManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // 残り時間を取得する
    float GetRemainingTime() const { return remainingTime; }

    // 設定時間
    float GetMaxTime() const { return maxTime; }

    // ゲームのステートをリセットする
    void Reset();

    // タイムアップ
    bool IsTimeUp() const
    {
        return remainingTime <= 0.0f;
    }

    // ゲーム終了処理
    void EndGame();

    // ゲーム開始処理
    void StartGame();

    // ゲームの入力処理状態を設定する
    void SetGameInputEnabled(bool enabled) { gameInputEnabled = enabled; }

    // ゲームの入力処理状態を取得する
    bool IsGameInputEnabled() const { return gameInputEnabled; }
private:
    float maxTime = 0.0f;      // 50秒
    float remainingTime = 50.0f;

    bool isGameEnded = false; // ゲームが終わったかどうか
    bool isGameRunning = false; // ゲーム開始かどうか


    // ゲームの入力を許可するかどうかのフラグ
    bool gameInputEnabled = true;

};