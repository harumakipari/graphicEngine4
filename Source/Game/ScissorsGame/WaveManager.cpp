#include "pch.h"

#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
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

void WaveManager::SetWaves(int stageId)
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

    SpawnBossIfNeeded(stage);
#endif // 0
}

void WaveManager::Update(float deltaTime)
{
    if (waveState == WaveState::Ready)
    {
        startTimer += deltaTime;

        if (startTimer < 0.5f)
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

        //  予告
        if (!state.previewed && timer >= s.delay)
        {
            SpawnPreviewEffect(s.position);
            state.previewed = true;
        }

        //  実際のスポーン遅らせる
        if (!state.spawned && timer >= s.delay + s.spawnDelay)
        {
            SpawnEnemy(s.position, s.type, s.isBig, s.speed, s.dir);
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

    bool isLastWave = (currentWave == waves.size() - 1);

    if (isLastWave)
    {
        static float time = 0.0f;
        time += deltaTime;
        if (time >= 1.0f)
        {
            OnLastEnemySpawned(); //  最終Waveだけ
        }

    }


    bool shouldGoNextWave = false; // 次のウェーブに行けるかどうか
    if (wave.requiredKills >= 0)
    {// キル数指定がある場合
        if (killCount >= wave.requiredKills)
        {
            shouldGoNextWave = true;
        }
    }
    else if (wave.waitForClear)
    {// 待つ条件がある場合
        if (allSpawned && AllEnemiesDead())
        {// 敵が全て出現　かつ　敵が全て死亡したら
            shouldGoNextWave = true;
        }
    }
    else
    {// 待つ条件がない場合
        if (allSpawned)
        {// 敵が全て出現したら
            shouldGoNextWave = true;
        }
    }

    if (shouldGoNextWave)
    {// 次のウェーブに行けたら、
        currentWave++;
        timer = 0;
        killCount = 0; // キル数をリセット
        hasSpawnedAnyEnemy = false;

        if (currentWave < waves.size())
        {
            spawnStates.clear();
            spawnStates.resize(waves[currentWave].spawns.size());
        }
    }

}

// 仮の敵を生成する関数
void WaveManager::SpawnEnemy(
    const DirectX::XMFLOAT3& pos,
    YarnEnemyType type, bool isBig,
    float speed, const DirectX::XMFLOAT3& dir)
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
}

// ステージ全体の最後のWaveの、最後の1体
void WaveManager::OnLastEnemySpawned()
{
    auto audioActor = GetOwnerScene()->GetActorManager()->GetActorByName("Audio");
    auto audioComp = audioActor->GetComponent<CoreAudioSourceComponent>();
    audioComp->SetPitch(1.2f);
}

// 必要ならボスを生成する
void WaveManager::SpawnBossIfNeeded(const StageData& stageData) const
{
    if (!stageData.bossData.hasBoss) return;

    const auto& b = stageData.bossData;

    Transform tr(b.position, { 0,180,0 }, { 1,1,1 });

    auto boss = GetOwnerScene()->GetActorManager()
        ->CreateAndRegisterActorWithTransform<RabbitBossEnemyActor>("boss", tr);
}
