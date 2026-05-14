#include "pch.h"

#include "BossSpawner.h"
#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
#include "ScissorsGameManager.h"
#include "StageLoader.h"
#include "WaveManagaer.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
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
    hasBossStage = false;  // ボスステージかどうか
    hasEndedGame = false;  // ゲームを終了終了条件を満たしているかどうか
    waveStarted = false;

    // 登場エフェクト用のコンポーネントを追加
    spawnEffectComponent = this->AddComponent<class ParticleComponent>(parentName);
    spawnEffectComponent->Load("./Data/Effect/Files/ScissorsGameCloudEffect.json");
}

void WaveManager::SetWaves(STAGE_NAME stageId)
{
#if 1
    auto stage = StageLoader::Load(stageId);
    waves = stage.waves;

    currentWave = 0;
    timer = 0.0f;
    killCount = 0;
    spawnIndex = 0;
    hasSpawnedAnyEnemy = false;

    spawnStates.clear();

    if (!waves.empty())
    {
        spawnStates.resize(waves[currentWave].spawns.size());
    }

    // ボスステージかどうか
    hasBossStage = stage.bossData.hasBoss;

    SpawnBossIfNeeded(stage);
#endif // 0
}

void WaveManager::Update(float deltaTime)
{

    switch (waveState)
    {
    case WaveState::Ready:
        UpdateReady(deltaTime);
        break;

    case WaveState::WaitingNextWave:
        UpdateWaiting(deltaTime);
        break;

    case WaveState::Spawning:
        UpdateSpawning(deltaTime);
        break;

    case WaveState::Finished:
        // 通常ステージ
        if (!hasBossStage && enemyCount == 0)
        {
            RequestGameClear();
        }
        break;
    }


    //bool isClearNow = enemyCount == 0 && !spawnStates.empty();
    //Logger::Log(U8("エネミーの数") + std::to_string(enemyCount));
    //std::string log = hasSpawnedAnyEnemy ? "hasSpawnedAnyEnemy is true!" : "hasSpawnedAnyEnemy is false!";
    //Logger::Log(log);
    //Logger::Log(U8("スポーンステートの数") + std::to_string(spawnStates.size()));


}

// 最初の待ち更新処理
void WaveManager::UpdateReady(float deltaTime)
{
    startTimer += deltaTime;

    if (startTimer < 0.5f)
    {
        return;
    }

    waveState = WaveState::Spawning;
    timer = 0.0f;
}

// wave 間の更新処理
void WaveManager::UpdateWaiting(float deltaTime)
{
    if (currentWave >= waves.size())
        return;

    auto& wave = waves[currentWave];

    startTimer += deltaTime;

    // ★ 全滅ならスキップ
    if (wave.skipDelayIfCleared && enemyCount == 0)
    {
        waveState = WaveState::Spawning;
        timer = 0.0f;
        return;
    }

    if (startTimer >= wave.startDelay)
    {
        waveState = WaveState::Spawning;
        timer = 0.0f;
    }
}

// 敵スポーンの更新処理
void WaveManager::UpdateSpawning(float deltaTime)
{
    if (currentWave >= waves.size())
        return;

    auto& wave = waves[currentWave];
    timer += deltaTime;

    // --- spawn処理 ---
    for (int i = 0; i < wave.spawns.size(); i++)
    {
        auto& s = wave.spawns[i];
        auto& state = spawnStates[i];

        if (!state.previewed && timer >= s.delay)
        {
            SpawnPreviewEffect(s.position);
            state.previewed = true;
        }

        if (!state.spawned && timer >= s.delay + s.spawnDelay)
        {
            // 敵が出てくる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE1/enemy_spawn1.wav", 1.0f);
            SpawnEnemy(s.position, s.type, s.isBig, s.speed, s.dir, s.isTied);
            state.spawned = true;
        }
    }

    waveStarted = true;


    // --- 全spawn確認 ---
    bool allSpawned = true;
    for (auto& s : spawnStates)
    {
        if (!s.spawned)
        {
            allSpawned = false;
            break;
        }
    }

    // --- Wave終了判定 ---
    bool shouldNext = false;

    if (wave.requiredKills > 0)
    {
        if (killCount >= wave.requiredKills)
            shouldNext = true;
    }
    else
    {

        if (wave.waitForClear)
        {
            if (enemyCount == 0)
                shouldNext = true;
        }
        else
        {
            shouldNext = true;
        }
    }

    if (allSpawned && shouldNext)
    {
        GoToNextWave();
    }
}

// 次のwaveに行くときの処理
void WaveManager::GoToNextWave()
{
    currentWave++;
    Logger::Log(U8("今のcurretWave") + std::to_string(currentWave));

    timer = 0;
    killCount = 0;
    hasSpawnedAnyEnemy = false;

    if (currentWave >= waves.size())
    {
        waveState = WaveState::Finished;
        return;
    }

    spawnStates.assign(waves[currentWave].spawns.size(), {});

    startTimer = 0.0f;
    waveState = WaveState::WaitingNextWave;
}

// 仮の敵を生成する関数
void WaveManager::SpawnEnemy(
    const DirectX::XMFLOAT3& pos,
    YarnEnemyType type, bool isBig,
    float speed, const DirectX::XMFLOAT3& dir, bool isTied)
{
    DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };
    if (isBig)
    {
        scale = { 1.0f, 1.0f, 1.0f };
    }
    else
    {
        scale = { 1.1f, 1.1f, 1.1f };
    }

    Transform tr(pos, { 0,180,0 }, scale);
    auto enemy = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<EnemyBase>("enemy", tr);
    enemy->SetMoveDirection(dir);

    auto size = isBig ? EnemyBase::Big : EnemyBase::Small;
    enemy->SetEnemySize(size);
    enemy->SetEnemyType(type);

    switch (type)
    {
    case YarnEnemyType::Static:
        enemy->SetBehavior(std::make_unique<StaticBehavior>());
        break;

    case YarnEnemyType::MoveHorizontal:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        enemy->SetMoveDirection({ 1,0,0 });
        break;

    case YarnEnemyType::MoveVertical:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        enemy->SetMoveDirection({ 0,0,1 });
        break;

    case YarnEnemyType::MoveLinear:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        break;

    case YarnEnemyType::WaveHorizontal:
        enemy->SetBehavior(std::make_unique<WaveHorizontalBehavior>());
        enemy->SetMoveDirection({ 1,0,0 });
        break;

    case YarnEnemyType::WaveVertical:
        enemy->SetBehavior(std::make_unique<WaveVerticalBehavior>());
        enemy->SetMoveDirection({ 0,0,1 });
        break;

    case YarnEnemyType::WaveMoveBehavior:
        enemy->SetBehavior(std::make_unique<WaveMoveBehavior>());
        break;

    case YarnEnemyType::ChasePlayer:
        enemy->SetBehavior(std::make_unique<ChaseBehavior>());
        break;
    case YarnEnemyType::RescueEnemy:
        enemy->SetBehavior(std::make_unique<RescueBehavior>());
        break;
    case YarnEnemyType::LongRangeAttack:
        enemy->SetAttack(std::make_unique<NeedleAttack>());
        break;
    }
    enemy->SetSpeed(speed);
    enemy->onDeath = [this, weak = std::weak_ptr(enemy)]()
        {
            if (auto e = weak.lock())
            {
                OnDeath(e.get());
            }
        };

    enemy->SetUpVisual();
    hasSpawnedAnyEnemy = true;
    aliveEnemies.push_back(enemy);

    if (isTied)
    {// 玉止めされていたら
        enemy->OnTied();
        enemy->SetBasePosition(pos);
        enemy->Face({ 0,0,-1 });
    }

    enemyCount++;
}


float DistanceFromLine(
    const DirectX::XMFLOAT3& p,
    const DirectX::XMFLOAT3& a,
    const DirectX::XMFLOAT3& b)
{
    XMVECTOR pa = XMLoadFloat3(&p) - XMLoadFloat3(&a);
    XMVECTOR ba = XMLoadFloat3(&b) - XMLoadFloat3(&a);

    float h = XMVectorGetX(XMVector3Dot(pa, ba)) /
        XMVectorGetX(XMVector3Dot(ba, ba));

    h = std::clamp(h, 0.0f, 1.0f);

    XMVECTOR proj = XMLoadFloat3(&a) + ba * h;

    return XMVectorGetX(XMVector3Length(XMLoadFloat3(&p) - proj));
}


void WaveManager::SpawnPreviewEffect(DirectX::XMFLOAT3 pos)
{
    DebugRender::DrawSphere(pos, 0.2f, { 1, 0.5f, 0, 1 }, 0.3f, true);
    if (spawnEffectComponent)
    {
        spawnEffectComponent->SetWorldLocationDirect(pos);
        spawnEffectComponent->Play();
    }
    spawnCount++;
}

// ステージ全体の最後のWaveの、最後の1体
void WaveManager::OnLastEnemySpawned()
{
#if 0
    auto audioActor = GetOwnerScene()->GetActorManager()->GetActorByName("Audio");
    auto audioComp = audioActor->GetComponent<CoreAudioSourceComponent>();
    audioComp->SetPitch(1.2f);

#endif // 0
}

// 必要ならボスを生成する
void WaveManager::SpawnBossIfNeeded(const StageData& stageData) const
{
    if (!stageData.bossData.hasBoss) return;

    const auto& b = stageData.bossData;

    Transform tr(b.position, { 0,180,0 }, { 1,1,1 });

    auto spawner = GetOwnerScene()->GetActorManager()
        ->CreateAndRegisterActorWithTransform<BossSpawner>("bossSpawner", tr);
    spawner->Activate();

    auto boss = GetOwnerScene()->GetActorManager()
        ->CreateAndRegisterActorWithTransform<RabbitBossEnemyActor>("boss", tr);

    boss->onDeath = [spawner]()
        {
            spawner->KillAllEnemies();
        };

}


// ゲーム終了を通知する関数
void WaveManager::RequestGameClear()
{
    if (hasEndedGame)
        return;

    hasEndedGame = true;

    auto gameManager =
        GetOwnerScene()->GetActorManager()
        ->GetActorOfType<ScissorsGameManager>();

    if (gameManager)
    {
        gameManager->EndGame();
    }

}