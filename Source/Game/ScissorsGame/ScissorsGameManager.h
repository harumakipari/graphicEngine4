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

    // 所要時間を取得する
    float GetRequiredTime() const { return gameTimer; }

    // ゲームのステートをリセットする
    void Reset();

    // ゲーム終了処理
    void EndGame(bool playerDead=false);

    // ゲーム開始処理　
    void StartGame();

    // ゲーム測定開始処理
    void StartTimer();

    // 終了演出を開始する
    void StartFinishPerform();

    // ゲームの入力処理状態を設定する
    void SetGameInputEnabled(bool enabled) { gameInputEnabled = enabled; }

    // ゲームの入力処理状態を取得する
    bool IsGameInputEnabled() const
    {
        int i = 0;
        return gameInputEnabled;
    }

    bool GetGameStart() { return isGameRunning; }

    bool GetGameEnd()const {return isGameEnded;}
private:
    float gameTimer = 0.0f; // ゲーム時間

    bool isGameEnded = false; // ゲームが終わったかどうか
    bool isGameRunning = false; // ゲーム開始かどうか


    // ゲームの入力を許可するかどうかのフラグ
    bool gameInputEnabled = true;

};