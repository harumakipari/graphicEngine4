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

    bool isTied = false; // 玉止めされているかどうか
};

struct SpawnRuntime
{
    bool previewed = false;
    bool spawned = false;
};

struct WaveData
{
    std::vector<SpawnData> spawns;

    bool waitForClear = false; // 全滅待ちかどうか
    int requiredKills = -1;// 必要キル数

    float startDelay = 0.0f; // 前のwaveから今のウェーブの間の待ち時間

    bool skipDelayIfCleared = true; // 敵が全滅していたら、次のwaveに行くかどうか
};

struct BossData
{
    bool hasBoss = false; // ボスを生成するかどうか
    DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };
};

struct StageData
{
    std::vector<WaveData> waves;
    BossData bossData;
};