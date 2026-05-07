#include "pch.h"
#include "BossSpawner.h"

#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
#include "Engine/Scene/Scene.h"

void BossSpawner::Initialize(const Transform& transform)
{
    std::string parentName = "bossSpawner";

    std::vector<SpawnEntry> table =
    {
        { YarnEnemyType::ChasePlayer,   75.0f, 2.0f, false, false },
        { YarnEnemyType::RescueEnemy,   10.0f, 3.0f, false, false }, // ハサミ speed=3
        { YarnEnemyType::ChasePlayer,   10.0f, 2.0f, true,  false }, // 大きい敵
        { YarnEnemyType::LongRangeAttack,5.0f, 2.0f, false, false }  // ハリネズミ
    };

    patterns =
    {
        // パターン1
        {
            {
                { {1,0,1},    { 1,0,-1 }, table },
                { {23,0,23},  { -1,0,-1 }, table }
            }
        },

        // パターン2
        {
            {
                { {1,0,23},   { 1,0,1 }, table },
                { {23,0,1},   { -1,0,1 }, table }
            }
        }
    };

    for (auto& pattern : patterns)
    {
        for (auto& p : pattern.points)
        {
            BuildBag(p);
        }
    }

    // 登場エフェクト用のコンポーネントを追加
    spawnEffectComponent = this->AddComponent<class ParticleComponent>(parentName);
    spawnEffectComponent->Load("./Data/Effect/Files/ScissorsGameCloudEffect.json");

}


void BossSpawner::Update(float deltaTime)
{
    if (!isActive) return;

    if (GetAliveEnemyCount() >= maxEnemies)
    {// 最大数を超えていたら
        return;
    }

    timer += deltaTime;

    if (timer >= interval)
    {

        timer = 0.0f;

        auto& pattern = patterns[currentPattern];

        for (auto& p : pattern.points)
        {
            //auto entry = SelectRandomEntry(p.table);
            auto entry = DrawFromBag(p);

            PendingSpawn ps;
            ps.point = p;
            ps.entry = entry;
            ps.startTime = 0.0f;

            pendingSpawns.push_back(ps);
        }
        // パターン切り替え
        currentPattern++;
        if (currentPattern >= patterns.size())
        {
            currentPattern = 0;
        }
    }


    // --- 予約されたspawn処理 ---
    for (auto& s : pendingSpawns)
    {
        s.startTime += deltaTime;

        // 予告
        if (!s.previewed && s.startTime >= previewDelay)
        {
            SpawnPreviewEffect(s.point.position);
            s.previewed = true;
        }

        // 実際の出現
        if (!s.spawned && s.startTime >= previewDelay + spawnDelay)
        {
            SpawnEnemy(
                s.point.position,
                s.entry.type,
                s.entry.isBig,
                s.entry.speed,
                s.point.direction,
                s.entry.isTied
            );

            s.spawned = true;
        }
    }

    pendingSpawns.erase(
        std::remove_if(pendingSpawns.begin(), pendingSpawns.end(),
            [](const PendingSpawn& s)
            {
                return s.spawned;
            }),
        pendingSpawns.end());
}

// 敵の種類を選択する 重み付きのランダム
SpawnEntry  BossSpawner::SelectRandomEntry(const std::vector<SpawnEntry>& table)
{
    float total = 0.0f;

    for (auto& e : table)
    {
        total += e.weight;
    }

    float r = MathHelper::RandomRange(0.0f, total);

    float accum = 0.0f;

    for (auto& e : table)
    {
        accum += e.weight;

        if (r <= accum)
            return e;
    }

    return table.back();
}

// 仮の敵を生成する関数
void BossSpawner::SpawnEnemy(
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
    // 生き残っている敵として登録する
    aliveEnemies.push_back(enemy);


    if (isTied)
    {// 玉止めされていたら
        enemy->OnTied();
        enemy->SetBasePosition(pos);
        enemy->Face({ 0,0,1 });
    }

}

// 生き残っている敵の数
int BossSpawner::GetAliveEnemyCount()
{
    aliveEnemies.erase(
        std::remove_if(aliveEnemies.begin(), aliveEnemies.end(),
            [](auto& w) { return w.expired(); }),
        aliveEnemies.end());

    return static_cast<int>(aliveEnemies.size());
}

// 敵が死亡した時に呼ぶ関数として登録する関数
void BossSpawner::OnDeath(EnemyBase* enemy)
{
    killCount++;

    aliveEnemies.erase(
        std::remove_if(aliveEnemies.begin(), aliveEnemies.end(),
            [enemy](const std::weak_ptr<EnemyBase>& weakEnemy)
            {
                if (auto e = weakEnemy.lock())
                {
                    return e.get() == enemy;
                }
                return false;
            }),
        aliveEnemies.end());
}

void BossSpawner::KillAllEnemies()
{
    int index = 0;
    auto enemies = GetOwnerScene()->GetActorManager()->GetActorsOfType<EnemyBase>();
    for (auto& enemy : enemies)
    {
        if (auto boss = std::dynamic_pointer_cast<RabbitBossEnemyActor>(enemy))
        {// ボスだったら、
            continue;
        }
        if (enemy->IsDead())
        {
            return;
        }
        enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
        enemy->CallDeath(false); // 死亡演出開始処理
        // 死亡演出に遅延を入れる
        enemy->SetDelayBeforeKnockback(index * 0.08f);
        index++;
    }

    aliveEnemies.clear();
}

// 出現エフェクトを生成
void BossSpawner::SpawnPreviewEffect(DirectX::XMFLOAT3 pos)
{
    if (spawnEffectComponent)
    {
        spawnEffectComponent->SetWorldLocationDirect(pos);
        spawnEffectComponent->Play();
    }
}

// 出現のバッグを生成する
void BossSpawner::RefillSpawnBag()
{
    spawnBag.clear();

    // 15体 通常追跡
    for (int i = 0; i < 15; i++)
    {
        spawnBag.push_back({
            YarnEnemyType::ChasePlayer,
            2.0f,
            false,
            false
            });
    }

    // 2体 Rescue
    for (int i = 0; i < 2; i++)
    {
        spawnBag.push_back({
            YarnEnemyType::RescueEnemy,
            3.0f,
            false,
            false
            });
    }

    // 2体 Big
    for (int i = 0; i < 2; i++)
    {
        spawnBag.push_back({
            YarnEnemyType::ChasePlayer,
            2.0f,
            true,
            false
            });
    }

    // 1体 ハリネズミ
    spawnBag.push_back({
        YarnEnemyType::LongRangeAttack,
        2.0f,
        false,
        false
        });

    std::shuffle(spawnBag.begin(), spawnBag.end(), rng);
}

// 敵の生成の袋を生成する
void BossSpawner::BuildBag(BossSpawnPoint& point)
{
    point.bag.clear();

    const float scale = 1.0f; // ← 小さめ袋（超重要）

    for (auto& e : point.table)
    {
        int count = std::max<int>(1, static_cast<int>(e.weight * scale));

        for (int i = 0; i < count; i++)
        {
            point.bag.push_back(e);
        }
    }

    std::shuffle(point.bag.begin(), point.bag.end(), std::mt19937{ std::random_device{}() });
    point.bagIndex = 0;
}

// 袋から取り出す関数
SpawnEntry BossSpawner::DrawFromBag(BossSpawnPoint& point)
{
    if (point.bag.empty())
    {
        BuildBag(point);
    }

    if (point.bagIndex >= point.bag.size())
    {
        std::shuffle(point.bag.begin(), point.bag.end(), rng);
        point.bagIndex = 0;
    }

    return point.bag[point.bagIndex++];
}