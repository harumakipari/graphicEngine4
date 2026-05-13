#pragma once

#include "EnemyScoreData.h"
#include "ComboSystem.h"
#include "ResultData.h"

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

        // 最大コンボ更新
        resultData.maxCombo = std::max<int>(resultData.maxCombo, combo.GetComboCount());

        // 総スコア更新
        resultData.totalScore = totalScore;
    }

    static int ProcessHit(const EnemyScoreData& data, bool isKilled)
    {
        combo.AddHit();

        float multiplier = combo.GetMultiplier();

        int score = calculator.CalculateScore(data, multiplier, isKilled);

        resultData.enemyScore += score;

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

    // 反射ボーナススコアを追加する関数
    static void AddReflectionBonus(int reflectionBonus)
    {
        int bonus = reflectionBonus ;

        if (bonus <= 0)
        {
            return;
        }

        totalScore += bonus;

        resultData.reflectionBonusScore += bonus;
    }

    // ５体ボーナスを追加
    static void AddDashBonus(int dashBonus)
    {
        int bonus = dashBonus;

        if (bonus <= 0)
        {
            return;
        }

        totalScore += bonus;

        resultData.dashBonusScore += bonus;
    }

    // 残ライフを記録
    static void AddLifeBonus(const int hp)
    {
        resultData.remainHp = hp;
    }

    // 所要時間を記録
    static void RecordGameTime(const float gameTime)
    {
        resultData.gameTimer = gameTime;
    }

    // リザルトデータを取得する
    static const ResultData& GetResultStats()
    {
        return resultData;
    }

    // 遊んでいるステージを設定する
    static void RecordStageName(const STAGE_NAME stageName)
    {
        resultData.stageName = stageName;
    }


    // 全てをリセットする
    static void Reset()
    {
        totalScore = 0;
        resultData = {};
        ResetCombo();
    }

private:
    static inline  ComboSystem combo;
    static inline  ScoreCalculator calculator;
    static inline  int totalScore = 0;
    static inline ResultData resultData = {};
};