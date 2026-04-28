#include "pch.h"

#include "EnemyBase.h"
#include "StageLoader.h"
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

    if (!waves.empty())
    {
        spawnStates.resize(waves[currentWave].spawns.size());
    }
#endif // 0
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
    Transform tr(pos, { 0,180,0 }, { 1.0f,1.0f,1.0f });
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

    case YarnEnemyType::WaveHorizontal:
        enemy->SetBehavior(std::make_unique<WaveHorizontalBehavior>());
        enemy->SetMoveDirection({ 1,0,0 });
        break;

    case YarnEnemyType::WaveVertical:
        enemy->SetBehavior(std::make_unique<WaveVerticalBehavior>());
        enemy->SetMoveDirection({ 0,0,1 });
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

void WaveManager::CheckLine(
    const std::vector<ScissorsGameEnemyBase*>& enemies,
    std::pair<int, int> dir)
{
    std::map<int, std::vector<ScissorsGameEnemyBase*>> groups;

    for (auto* e : enemies)
    {
        auto p = e->GetPosition();

        float raw;

        if (dir == std::make_pair(1, 0))        raw = p.z;
        else if (dir == std::make_pair(0, 1))   raw = p.x;
        else if (dir == std::make_pair(1, 1))   raw = p.x - p.z;
        else                                    raw = p.x + p.z;

        // ここがポイント
        int key = (int)round(raw / 3.0f);

        groups[key].push_back(e);
    }

    for (auto& [key, line] : groups)
    {
        if (line.size() < 5) continue;

        // 並び順にソート
        std::sort(line.begin(), line.end(),
            [dir](auto* a, auto* b)
            {
                if (dir.first != 0)
                    return a->GetPosition().x < b->GetPosition().x;
                else
                    return a->GetPosition().z < b->GetPosition().z;
            });

        // 可視化
        for (int i = 0; i < line.size() - 1; i++)
        {
            DebugRender::DrawLine(
                line[i]->GetPosition(),
                line[i + 1]->GetPosition(),
                { 1,1,0,1 });
        }

        for (auto* e : line)
        {
            e->SetHighlight(true);
        }
    }
}

// ラインを検出後
void WaveManager::OnLineDetected(const std::vector<std::weak_ptr<ScissorsGameEnemyBase>>& line)
{
    // 線描画
    for (int i = 0; i < line.size() - 1; i++)
    {
        DebugRender::DrawLine(
            line[i].lock()->GetPosition(),
            line[i + 1].lock()->GetPosition(),
            { 1,1,0,1 }
        );
    }

    // 光らせる
    for (auto e : line)
    {
        e.lock()->SetHighlight(true);
    }
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
