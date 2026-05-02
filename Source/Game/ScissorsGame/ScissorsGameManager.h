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

    // 提出失敗時の処理
    void OnSubmitMiss();

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

    // ゲームの入力処理状態を設定する
    void SetGameInputEnabled(bool enabled) { gameInputEnabled = enabled; }

    // ゲームの入力処理状態を取得する
    bool IsGameInputEnabled() const { return gameInputEnabled; }

    // フィーバーに入った瞬間を取得する
    bool ConsumeFeverMode();

    // フィーバーに入った瞬間を取得する
    bool ConsumeFeverWordAppear();
private:
    // コンボを加算する
    void AddCombo();
private:
    float totalScore = 0.0f;
    int   combo = 0;

    float maxTime = 0.0f;      // 50秒
    float remainingTime = 50.0f;

    float satisfaction = 0.0f;  // 満足度

    bool isGameEnded = false; // ゲームが終わったかどうか
    bool isGameRunning = false; // ゲーム開始かどうか

    float feverGauge = 0.0f;
    float feverGaugeMax = 100.0f;

    float feverTime = 8.0f; // フィーバーの時間
    float feverRemainingTime = 0.0f;

    int feverTriggerCombo = 4;  // フィーバーになるまでに提出する個数

    float bgmPitch = 1.0f;

    // ゲームの入力を許可するかどうかのフラグ
    bool gameInputEnabled = true;

    // フィーバーに入った瞬間を取得する
    bool justFeverMode = false;

    // フィーバーに入った瞬間の文字演出
    bool justAppearWord = false;
};