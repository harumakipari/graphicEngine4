#include "pch.h"
#include "BossSpawner.h"

#include "EnemyBase.h"
#include "Engine/Scene/Scene.h"

void BossSpawner::Initialize(const Transform& transform)
{
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

}


void BossSpawner::Update(float deltaTime)
{
    if (!isActive) return;

    if (GetAliveEnemyCount() >= maxEnemies)
    {// 最大数を超えていたら
        return;
    }

    timer += deltaTime;

    if (timer < interval) return;

    timer = 0.0f;

    auto& pattern = patterns[currentPattern];

    for (auto& p : pattern.points)
    {
        auto entry = SelectRandomEntry(p.table);

        SpawnEnemy(
            p.position,
            entry.type,
            entry.isBig,
            entry.speed,
            p.direction,
            entry.isTied
        );
    }

    // パターン切り替え
    currentPattern++;
    if (currentPattern >= patterns.size())
    {
        currentPattern = 0;
    }
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

    // 生き残っている敵として登録する
    aliveEnemies.push_back(enemy);

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
            }
        };

    enemy->SetUpVisual();

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

void BossSpawner::KillAllEnemies()
{
    int index = 0;
    for (auto& w : aliveEnemies)
    {
        if (auto enemy = w.lock())
        {
            enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
            enemy->CallDeath(false); // 死亡演出開始処理
            // 死亡演出に遅延を入れる
            enemy->SetDelayBeforeKnockback(index * 0.08f);
            index++;
        }
    }

    aliveEnemies.clear();
}
