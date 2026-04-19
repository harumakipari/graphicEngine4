#include "pch.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void WaveManager::Initialize(const Transform& transform)
{
    waves =
    {
       {
            // Wave 1（慣らし）
            {
                { {6,0,8}, YarnEnemyType::Static, 0.0f },
                { {4,0,8}, YarnEnemyType::Static, 0.5f },
                { {8,0,8}, YarnEnemyType::Static, 1.0f },
            },
            true
        },

        {
            // Wave 2（ちょい圧）
            {
                { {0,0,8}, YarnEnemyType::MoveHorizontal, 0.0f },
                { {12,0,8}, YarnEnemyType::MoveHorizontal, 0.5f },
                { {6,0,0}, YarnEnemyType::MoveVertical, 1.0f },
            },
            false
        },

        {
            // Wave 3（追い込み）
            {
                { {6,0,10}, YarnEnemyType::ChasePlayer, 0.0f },
                { {5,0,9}, YarnEnemyType::ChasePlayer, 0.3f },
                { {7,0,9}, YarnEnemyType::ChasePlayer, 0.3f },
            },
            true
        }
    };
}


void WaveManager::Update(float deltaTime)
{
    if (currentWave >= waves.size()) return;

    auto& wave = waves[currentWave];
    timer += deltaTime;

    // スポーン処理
    while (spawnIndex < wave.spawns.size() &&
        timer >= wave.spawns[spawnIndex].delay)
    {
        auto& s = wave.spawns[spawnIndex];

        if (s.isBig)
            SpawnBigEnemy(s.position, s.type, s.speed, s.dir);
        else
            SpawnEnemy(s.position, s.type, s.speed, s.dir);

        spawnIndex++;
    }

    // 次のWaveへ
    if (wave.waitForClear)
    {
        if (AllEnemiesDead())
        {
            currentWave++;
            timer = 0;
            spawnIndex = 0;
        }
    }
    else
    {
        if (spawnIndex >= wave.spawns.size())
        {
            currentWave++;
            timer = 0;
            spawnIndex = 0;
        }
    }
}

// 仮の敵を生成する関数
void WaveManager::SpawnEnemy(
    const DirectX::XMFLOAT3& pos,
    YarnEnemyType type,
    float speed, const DirectX::XMFLOAT3& dir)
{
    Transform tr(pos, { 0,0,0 }, { 0.5f,0.5f,0.5f });
    auto enemy = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<YarnEnemyActor>("enemy", tr);
    enemy->SetMoveDirection(dir);
    enemy->SetType(type);
    enemy->SetSpeed(speed);
    enemy->onDeath = [this]()
        {
            enemyCount--;
        };


    enemyCount++;
}

// 仮の敵を生成する関数
void WaveManager::SpawnBigEnemy(
    const DirectX::XMFLOAT3& pos,
    YarnEnemyType type,
    float speed, const DirectX::XMFLOAT3& dir)
{
    Transform tr(pos, { 0,0,0 }, { 1.0f,1.0f,1.0f });
    auto enemy = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<BigYarnEnemyActor>("enemy", tr);
    enemy->SetMoveDirection(dir);
    enemy->SetType(type);
    enemy->SetSpeed(speed);
    enemy->onDeath = [this]()
        {
            enemyCount--;
        };

    enemyCount++;
}

