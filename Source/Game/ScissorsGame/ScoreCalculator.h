#pragma once

#include "EnemyScoreData.h"
#include "ComboSystem.h"

class ScoreCalculator
{
public:
    // スコアを計算する関数
    int CalculateScore(
        const EnemyScoreData& data,
        float multiplier,
        bool isKilled)
    {
        int score = static_cast<int>(data.baseScore * multiplier);

        if (isKilled)
        {
            score += data.killBonus;
        }

        return score;
    }
};


class ScoreSystem
{
public:
    static void Update(float deltaTime)
    {
        combo.Update(deltaTime);
    }

    static int ProcessHit(const EnemyScoreData& data, bool isKilled)
    {
        combo.AddHit();

        float multiplier = combo.GetMultiplier();

        int score = calculator.CalculateScore(data, multiplier, isKilled);

        totalScore += score;

        // ログにスコア、コンボ数、倍率を出力
        Logger::Log("Hit! Score: " + std::to_string(score) + " (Total: " + std::to_string(totalScore) + ", Combo: " + std::to_string(combo.GetComboCount()) + ", Multiplier: " + std::to_string(multiplier) + ")");

        return score;
    }

    static void ResetCombo()
    {
        combo.Reset();
    }

    static int GetCombo() { return combo.GetComboCount(); }

    // 総スコアを取得する関数
    static int GetTotalScore() { return totalScore; }

    // ボーナススコアを追加する関数
    static void AddBonusScore(const int score)
    {
        totalScore += score;
        Logger::Log("Bonus Score: " + std::to_string(score) +
            " (Total: " + std::to_string(totalScore) + ")");
    }

    // 全てをリセットする
    static void Reset()
    {
        totalScore = 0;
        ResetCombo();
    }

private:
    static inline  ComboSystem combo;
    static inline  ScoreCalculator calculator;
    static inline  int totalScore = 0;
};