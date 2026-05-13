#pragma once
#include "StageData.h"


struct ResultData
{
    // 反射ボーナス
    int reflectionBonusScore = 0;
    // ダッシュボーナス
    int dashBonusScore = 0;
    // 残っているHP
    int remainHp = 0;
    // 最大コンボ数
    int maxCombo = 0;
    // 敵を倒したスコア
    int enemyScore = 0;
    // 総スコア
    int totalScore = 0;
    // 所要時間
    float gameTimer = 0.0f;
    // 遊んだステージ名
    STAGE_NAME stageName = STAGE_NAME::TUTORIAL;
};
