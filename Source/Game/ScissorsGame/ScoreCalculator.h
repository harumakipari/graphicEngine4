#pragma once

#include <magic_enum.hpp>

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
    static void Update(float deltaTime, bool isDashing)
    {
        combo.Update(deltaTime, isDashing);

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
        int bonus = reflectionBonus;

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

    // タイムクリアボーナスを計算する関数
    static int CalculateTimeClearBonus()
    {
        auto it = stageTimeTable.find(resultData.stageName);

        if (it == stageTimeTable.end())
            return 0;

        const StageTimeData& config = it->second;

        Logger::Log(std::string(magic_enum::enum_name(resultData.stageName)) + U8("目標時間：") + std::to_string(config.targetTime));

        // 目標タイム以内ならボーナス
        if (resultData.gameTimer <= config.targetTime)
        {
            return 1000;
        }

        return 0;
    }

    // タイムクリアかどうかを判定する関数
    static bool IsTimeClear()
    {
        auto it = stageTimeTable.find(resultData.stageName);

        if (it == stageTimeTable.end())
            return false;

        const StageTimeData& config = it->second;

        return resultData.gameTimer <= config.targetTime;
    }

    // クリアまでの残り時間を取得する関数
    static float GetRemainTimeToClear()
    {
        auto it = stageTimeTable.find(resultData.stageName);

        if (it == stageTimeTable.end())
            return 0.0f;

        const StageTimeData& config = it->second;

        // どれだけ超過したか
        float remain = resultData.gameTimer - config.targetTime;

        return (remain > 0.0f) ? remain : 0.0f;
    }

    // 任意のスコアを追加
    static void AddScore(int score)
    {
        if (score <= 0)
        {
            return;
        }

        totalScore += score;

        resultData.enemyScore += score;
    }

    // 全てをリセットする
    static void Reset()
    {
        totalScore = 0;
        resultData = {};
        ResetCombo();
    }

    // ボス撃破時間ボーナス
    static int CalculateBossTimeBonus(float clearTime)
    {
        // 1分以内
        if (clearTime <= 60.0f)
        {
            return 10000;
        }

        // 1分30秒以内
        if (clearTime <= 90.0f)
        {
            return 7500;
        }

        // 1分45秒以内
        if (clearTime <= 105.0f)
        {
            return 6000;
        }

        // 2分以内
        if (clearTime <= 120.0f)
        {
            return 4000;
        }

        // それ以降
        return 2000;
    }
public:
    static inline std::unordered_map<STAGE_NAME, StageTimeData> stageTimeTable =
    {
        { STAGE_NAME::TUTORIAL,      { 999.0f, 0 } },   // チュートリアルはボーナス無しでもOK
        { STAGE_NAME::FIRST,         { 50.0f,  0 } },
        { STAGE_NAME::BOBBIN_FIRST,  { 60.0f,  0 } },
        { STAGE_NAME::REFLECT_WALL,  { 55.0f,  0 } },
        { STAGE_NAME::BOBBIN_SECOND, { 990.0f,  0 } },
        { STAGE_NAME::DIFFICULT,     { 80.0f,  0 } },
        { STAGE_NAME::BOSS,          { 105.0f, 0 } },
    };


private:
    static inline  ComboSystem combo;
    static inline  ScoreCalculator calculator;
    static inline  int totalScore = 0;
    static inline ResultData resultData = {};
};