#include "pch.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void WaveManager::Initialize(const Transform& transform)
{
    std::string parentName = "waveManager";

    // 初期化
    currentWave = 0;
    timer = 0;
    spawnIndex = 0;
    enemyCount = 0;
    spawnStates.clear();
    waveState = WaveState::Ready;
    startTimer = 0.0f;

#if 1
    waves =
    {
       {
            // Wave 1
            {
                { {3,0,19}, YarnEnemyType::Static, 0.0f },
            },
            false
        },

        {
            // Wave 2（ちょい圧）
            {
                { {21,0,15}, YarnEnemyType::MoveHorizontal, 3.0f },
                { {0,0,6}, YarnEnemyType::MoveHorizontal, 6.0f },
            },
            false
        },

        {
            // Wave 3（追い込み）
            {
                { {21,0,11}, YarnEnemyType::MoveHorizontal, 4.0f },
                { {19,0,12}, YarnEnemyType::MoveHorizontal, 3.5f ,true},
                { {21,0,13}, YarnEnemyType::MoveHorizontal, 4.0f },
            },
            false
        },

        {
            // Wave 3（追い込み）
            {
                { {0,0,4}, YarnEnemyType::MoveHorizontal, 4.0f },
                { {1,0,5}, YarnEnemyType::MoveHorizontal, 3.5f ,true},
                { {0,0,6}, YarnEnemyType::MoveHorizontal, 4.0f },
            },
            false
        },

        {
            // Wave 3（追い込み）
            {
                { {14,0,21}, YarnEnemyType::MoveVertical, 4.0f },
                { {16,0,21}, YarnEnemyType::MoveVertical, 4.0f },
                { {4,0,0}, YarnEnemyType::MoveVertical, 4.5f },
                { {6,0,0}, YarnEnemyType::MoveVertical, 4.5f },
            },
            false
        }
    };
    spawnStates.resize(waves[currentWave].spawns.size());

#endif // 0

#if 0
    float alignTime = 5.0f;

    auto line = MakeDiagonalLine({ 3,0,3 }, { 0,0,1 }, 5, 2.0f);

    for (int i = 0; i < line.size(); i++)
    {
        float randomSpeed = MathHelper::RandomRange(1.0f, 5.0f);
        float spawnTime = i * 0.5f; // ← バラして出す

        auto spawnPos = CalcAlignedSpawnPos(
            line[i],        // 最終的に揃う位置
            { 1,0,0 },        // 移動方向
            randomSpeed,           // speed
            spawnTime,
            alignTime
        );


        // delay = spawnTime にする
        SpawnEnemy(
            spawnPos,
            YarnEnemyType::MoveHorizontal,
            randomSpeed,
            { 1,0,0 }
        );
    }
#endif // 0

    // 登場エフェクト用のコンポーネントを追加
    spawnEffectComponent = this->AddComponent<class ParticleComponent>(parentName);
    spawnEffectComponent->Load("./Data/Effect/Files/ScissorsGameCloudEffect.json");

}


void WaveManager::Update(float deltaTime)
{
    if (waveState == WaveState::Ready)
    {
        startTimer += deltaTime;

        if (startTimer < 3.0f)
        {
            return;
        }

        waveState = WaveState::Spawning;
        timer = 0.0f;
    }

    if (currentWave >= waves.size()) return;

    auto& wave = waves[currentWave];
    timer += deltaTime;

#if 0
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
#else
    for (int i = 0; i < wave.spawns.size(); i++)
    {
        auto& s = wave.spawns[i];
        auto& state = spawnStates[i];

        // ① 予告（ピカピカ＋煙）
        if (!state.previewed && timer >= s.delay)
        {
            SpawnPreviewEffect(s.position);
            state.previewed = true;
        }

        // ② 実際のスポーン（遅らせる！）
        if (!state.spawned && timer >= s.delay + s.spawnDelay)
        {
            if (s.isBig)
                SpawnBigEnemy(s.position, s.type, s.speed, s.dir);
            else
                SpawnEnemy(s.position, s.type, s.speed, s.dir);

            state.spawned = true;
        }
    }
#endif // 0

    bool allSpawned = true;

    for (auto& s : spawnStates)
    {
        if (!s.spawned)
        {
            allSpawned = false;
            break;
        }
    }

    // 次のWaveへ
    if (wave.waitForClear)
    {
        if (allSpawned && AllEnemiesDead())
        {
            currentWave++;
            timer = 0;
            hasSpawnedAnyEnemy = false;

            if (currentWave < waves.size())
            {
                spawnStates.clear();
                spawnStates.resize(waves[currentWave].spawns.size());
            }
        }
    }
    else
    {

        if (allSpawned)
        {
            currentWave++;
            timer = 0;

            if (currentWave < waves.size())
            {
                spawnStates.clear();
                spawnStates.resize(waves[currentWave].spawns.size());
            }
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
    hasSpawnedAnyEnemy = true;

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
    hasSpawnedAnyEnemy = true;
    enemyCount++;
}




void WaveManager::SpawnPreviewEffect(DirectX::XMFLOAT3 pos)
{
    DebugRender::DrawSphere(pos, 0.2f, { 1, 0.5f, 0, 1 }, 0.3f, true);
    if (spawnEffectComponent)
    {
        spawnEffectComponent->SetWorldLocationDirect(pos);
        spawnEffectComponent->Play();
    }
}
