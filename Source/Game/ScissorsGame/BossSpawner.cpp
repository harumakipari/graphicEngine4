#include "pch.h"
#include "BossSpawner.h"

#include "EnemyBase.h"
#include "Engine/Scene/Scene.h"

void BossSpawner::Initialize(const Transform& transform)
{
    std::vector<SpawnEntry> table =
    {
        { YarnEnemyType::ChasePlayer,   75.0f, 2.0f, false, false },
        { YarnEnemyType::RescueEnemy,   10.0f, 3.0f, false, false }, // ƒnƒTƒ~ speed=3
        { YarnEnemyType::ChasePlayer,   10.0f, 2.0f, true,  false }, // ‘å‚«‚¢“G
        { YarnEnemyType::LongRangeAttack,5.0f, 2.0f, false, false }  // ƒnƒŠƒlƒYƒ~
    };

    patterns =
    {
        // ƒpƒ^[ƒ“1
        {
            {
                { {1,0,1},    { 1,0,-1 }, table },
                { {23,0,23},  { -1,0,-1 }, table }
            }
        },

        // ƒpƒ^[ƒ“2
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
    {// Å‘å”‚ğ’´‚¦‚Ä‚¢‚½‚ç
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

    // ƒpƒ^[ƒ“Ø‚è‘Ö‚¦
    currentPattern++;
    if (currentPattern >= patterns.size())
    {
        currentPattern = 0;
    }
}

// “G‚Ìí—Ş‚ğ‘I‘ğ‚·‚é d‚İ•t‚«‚Ìƒ‰ƒ“ƒ_ƒ€
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

// ‰¼‚Ì“G‚ğ¶¬‚·‚éŠÖ”
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
    // ¶‚«c‚Á‚Ä‚¢‚é“G‚Æ‚µ‚Ä“o˜^‚·‚é
    aliveEnemies.push_back(enemy);


    if (isTied)
    {// ‹Ê~‚ß‚³‚ê‚Ä‚¢‚½‚ç
        enemy->OnTied();
        enemy->SetBasePosition(pos);
        enemy->Face({ 0,0,1 });
    }

}

// ¶‚«c‚Á‚Ä‚¢‚é“G‚Ì”
int BossSpawner::GetAliveEnemyCount()
{
    aliveEnemies.erase(
        std::remove_if(aliveEnemies.begin(), aliveEnemies.end(),
            [](auto& w) { return w.expired(); }),
        aliveEnemies.end());

    return static_cast<int>(aliveEnemies.size());
}

// “G‚ª€–S‚µ‚½‚ÉŒÄ‚ÔŠÖ”‚Æ‚µ‚Ä“o˜^‚·‚éŠÖ”
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
    for (auto& w : aliveEnemies)
    {
        if (auto enemy = w.lock())
        {
            enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
            enemy->CallDeath(false); // €–S‰‰oŠJnˆ—
            // €–S‰‰o‚É’x‰„‚ğ“ü‚ê‚é
            enemy->SetDelayBeforeKnockback(index * 0.08f);
            index++;
        }
    }

    aliveEnemies.clear();
}
