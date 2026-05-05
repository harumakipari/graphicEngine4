#pragma once
#include "EnemyScoreData.h"
#include "Core/Actor.h"

class EnemyBase;

struct SpawnEntry
{
    YarnEnemyType type;
    float weight; // 出現確率の重み

    float speed = 2.0f;
    bool isBig = false;
    bool isTied = false;
};

struct BossSpawnPoint
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 direction;
    std::vector<SpawnEntry> table;
};

struct BossSpawnPattern
{
    std::vector<BossSpawnPoint> points;
};

class BossSpawner :public Actor
{
public:
    explicit BossSpawner(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // 敵出現を開始する
    void Activate() { isActive = true; }

    // 生き残っている敵を全て死亡させる
    void KillAllEnemies();
private:
    // 敵の種類を選択する 重み付きのランダム
    SpawnEntry  SelectRandomEntry(const std::vector<SpawnEntry>& table);

    // 敵を出現させる
    void SpawnEnemy(
        const DirectX::XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed, const DirectX::XMFLOAT3& dir, bool isTied) ;

    // 生き残っている敵の数
    int GetAliveEnemyCount();
private:
    float timer = 0.0f;
    float interval = 5.0f; // 敵が出現する間隔秒数

    int maxEnemies = 20;    // 敵の同時最大出現数

    bool isActive = false;

    int currentPattern = 0;
    std::vector<BossSpawnPattern> patterns; // 敵出現
    std::vector<std::weak_ptr<EnemyBase>> aliveEnemies; //　生き残っている敵
};
