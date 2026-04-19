#pragma once
#include "Core/Actor.h"

#include "YarnEnemyActor.h"

struct SpawnData
{
    DirectX::XMFLOAT3 position;
    YarnEnemyType type;
    float delay; // Ç±ÇÃìGÇ™èoÇÈÇ‹Ç≈ÇÃéûä‘
    float speed;
    DirectX::XMFLOAT3 dir;
    bool isBig;
};

struct WaveData
{
    std::vector<SpawnData> spawns;

    bool waitForClear; // ëSñ≈ë“ÇøÇ©Ç«Ç§Ç©
};

class WaveManager :public Actor
{
public:
    explicit WaveManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    void SpawnEnemy(
        const DirectX::XMFLOAT3& pos,
        YarnEnemyType type,
        float speed = 2.0f, const DirectX::XMFLOAT3& dir = { 1,0,0 });

    void SpawnBigEnemy(
        const DirectX::XMFLOAT3& pos,
        YarnEnemyType type,
        float speed = 2.0f, const DirectX::XMFLOAT3& dir = { 1,0,0 });

    bool AllEnemiesDead() const
    {
        return enemyCount == 0;
    }

private:
    int currentWave = 0;
    float timer = 0.0f;
    int spawnIndex = 0;
    std::vector<WaveData> waves;

    int enemyCount = 0;
};