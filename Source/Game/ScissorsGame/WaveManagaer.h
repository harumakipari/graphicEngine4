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
};

class WaveManager :public Actor
{
public:
    enum class WaveState:uint8_t
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

    bool AllEnemiesDead() const
    {
        return enemyCount == 0;
    }

    // 出現エフェクトを生成
    void SpawnPreviewEffect(DirectX::XMFLOAT3 pos);
private:
    int currentWave = 0;  // 今のウェーブ
    float timer = 0.0f;
    int spawnIndex = 0;

    std::vector<WaveData> waves;
    std::vector<SpawnRuntime> spawnStates;

    int enemyCount = 0;

    std::shared_ptr<ParticleComponent> spawnEffectComponent; // 出現エフェクト用コンポーネント

    WaveState waveState = WaveState::Ready;
    bool hasSpawnedAnyEnemy = false; //敵がスポーンを開始したかどうか
    float startTimer = 0.0f;// wave１が始まるまでの時間
};