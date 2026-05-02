#pragma once
#include "EnemyScoreData.h"

struct SpawnData
{
    DirectX::XMFLOAT3 position;
    YarnEnemyType type;
    float delay; // この敵が出るまでの時間

    bool isBig = false;
    float speed = 2.0f;

    float spawnDelay = 1.3f; // ← 予告から出るまで

    DirectX::XMFLOAT3 dir = { 1,0,0 };
};

struct SpawnRuntime
{
    bool previewed = false;
    bool spawned = false;
};

struct WaveData
{
    std::vector<SpawnData> spawns;

    bool waitForClear; // 全滅待ちかどうか

    int requiredKills = -1; // 必要キル数
};

struct StageData
{
    std::vector<WaveData> waves;
};