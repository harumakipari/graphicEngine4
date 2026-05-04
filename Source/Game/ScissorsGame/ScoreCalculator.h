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
    void Update(float deltaTime)
    {
        combo.Update(deltaTime);
    }

    int ProcessHit(const EnemyScoreData& data, bool isKilled)
    {
        combo.AddHit();

        float multiplier = combo.GetMultiplier();

        int score = calculator.CalculateScore(data, multiplier, isKilled);

        totalScore += score;

        // ログにスコア、コンボ数、マルチプライヤーを出力
        Logger::Log("Hit! Score: " + std::to_string(score) + " (Total: " + std::to_string(totalScore) + ", Combo: " + std::to_string(combo.GetComboCount()) + ", Multiplier: " + std::to_string(multiplier) + ")");

        return score;
    }

    void ResetCombo()
    {
        combo.Reset();
    }

    int GetCombo()const { return combo.GetComboCount(); }

    // 総スコアを取得する関数
    int GetTotalScore() const { return totalScore; }

    // ボーナススコアを追加する関数
    void AddBonusScore(const int score)
    {
        totalScore += score;
        Logger::Log("Bonus Score: " + std::to_string(score) +
            " (Total: " + std::to_string(totalScore) + ")");
    }

private:
    ComboSystem combo;
    ScoreCalculator calculator;
    int totalScore = 0;
};