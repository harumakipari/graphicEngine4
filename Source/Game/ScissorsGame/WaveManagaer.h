#pragma once
#include "Core/Actor.h"

#include "YarnEnemyActor.h"
#include "Components/Effect/ParticleComponent.h"

struct SpawnData
{
    DirectX::XMFLOAT3 position;
    YarnEnemyType type;
    float delay; // この敵が出るまでの時間

    bool isBig = false;
    float spawnDelay = 1.0f; // ← 予告から出るまで

    float speed = 2.0f;
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

struct GridPos
{
    int x;
    int z;

    bool operator<(const GridPos& o) const
    {
        if (x != o.x) return x < o.x;
        return z < o.z;
    }
};


class WaveManager :public Actor
{
public:
    enum class WaveState :uint8_t
    {
        Ready,      // 3,2,1カウント中
        Spawning,   // 通常のWave処理
        Finished
    };
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

    // 出現位置の補正
    DirectX::XMFLOAT3 CalcAlignedSpawnPos(
        DirectX::XMFLOAT3 targetPos,
        DirectX::XMFLOAT3 dir,
        float speed,
        float spawnTime,
        float alignTime)
    {
        float moveTime = alignTime - spawnTime;

        return {
            targetPos.x - dir.x * speed * moveTime,
            targetPos.y,
            targetPos.z - dir.z * speed * moveTime
        };
    }

    bool AllEnemiesDead() const
    {
        return enemyCount == 0;
    }

    // 出現エフェクトを生成
    void SpawnPreviewEffect(DirectX::XMFLOAT3 pos);

    // ラインを作る関数
    std::vector<DirectX::XMFLOAT3> MakeDiagonalLine(
        DirectX::XMFLOAT3 start,
        DirectX::XMFLOAT3 dir,
        int count,
        float spacing)
    {
        std::vector<DirectX::XMFLOAT3> result;

        for (int i = 0; i < count; i++)
        {
            result.push_back({
                start.x + dir.x * spacing * i,
                start.y,
                start.z + dir.z * spacing * i
                });
        }

        return result;
    }

    // 敵が死んだときに呼ぶ関数として登録する関数
    void OnDeath(YarnEnemyActor* enemy)
    {
        enemyCount--;
        killCount++;

        aliveEnemies.erase(
            std::remove_if(aliveEnemies.begin(), aliveEnemies.end(),
                [enemy](const std::weak_ptr<ScissorsGameEnemyBase>& weakEnemy)
                {
                    if (auto e = weakEnemy.lock())
                    {
                        return e.get() == enemy;
                    }
                    return false;
                }),
            aliveEnemies.end());
    }

    // 直線判定
    void DetectLine();
    void CheckLine(const std::vector<ScissorsGameEnemyBase*>& enemies, std::pair<int, int> dir);

    // ラインを検出後
    void OnLineDetected(const std::vector<std::weak_ptr<ScissorsGameEnemyBase>>& line);

    GridPos ToGrid(const DirectX::XMFLOAT3& pos)
    {
        const float cellSize = 1.0f; // ← ここ重要

        return {
            (int)round(pos.x / cellSize),
            (int)round(pos.z / cellSize)
        };
    }

private:
    int currentWave = 0;  // 今のウェーブ
    float timer = 0.0f;
    int spawnIndex = 0;

    std::vector<WaveData> waves;
    std::vector<SpawnRuntime> spawnStates;

    int enemyCount = 0;
    int killCount = 0; // キルカウント

    std::shared_ptr<ParticleComponent> spawnEffectComponent; // 出現エフェクト用コンポーネント

    WaveState waveState = WaveState::Ready;
    bool hasSpawnedAnyEnemy = false; //敵がスポーンを開始したかどうか
    float startTimer = 0.0f;// wave１が始まるまでの時間

    std::vector<std::weak_ptr<ScissorsGameEnemyBase>> aliveEnemies;
    float lineCheckTimer = 0.0f;
};