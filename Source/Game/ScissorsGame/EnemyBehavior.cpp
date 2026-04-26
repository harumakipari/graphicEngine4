#include "pch.h"
#include "EnemyBehavior.h"
#include "EnemyBase.h"
#include "ScissorsGameState.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void StaticBehavior::Update(EnemyBase* e, float dt)
{
    e->Face({ 0.0f,0.0f,-1.0f });
}

void LinearBehavior::Update(EnemyBase* e, float deltaTime)
{
    DirectX::XMFLOAT3 pos = e->GetPosition();
    DirectX::XMFLOAT3 moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ÉXÉeÅ[ÉWí[Ç≈îΩì]
    if (pos.x < ScissorsGameState::stageMinX || pos.x > ScissorsGameState::stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX);
    }

    if (pos.z < ScissorsGameState::stageMinZ || pos.z > ScissorsGameState::stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ);
    }

    e->SetPosition(pos);
    e->SetMoveDirection(moveDirection);

    e->Face(moveDirection);
}


void WaveHorizontalBehavior::Update(EnemyBase* e, float dt)
{
    waveTime += dt;

    auto pos = e->GetPosition();

    auto start = e->GetStartPosition();
    auto moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    start.x += moveDirection.x * speed * dt;

    e->SetStartPosition(start);

    // îg
    pos.x = start.x;
    pos.z = start.z + sin(waveTime * waveFrequency) * waveAmplitude;

    e->SetPosition(pos);

    if (pos.x < ScissorsGameState::stageMinX)
    {
        pos.x = ScissorsGameState::stageMinX;
        start.x = pos.x;
        moveDirection.x = 1.0f;
    }
    else if (pos.x > ScissorsGameState::stageMaxX)
    {
        pos.x = ScissorsGameState::stageMaxX;
        start.x = pos.x;
        moveDirection.x = -1.0f;
    }

    e->SetMoveDirection(moveDirection);
    e->Face(moveDirection);
}

void WaveVerticalBehavior::Update(EnemyBase* e, float dt)
{
    waveTime += dt;

    auto pos = e->GetPosition();

    auto start = e->GetStartPosition();
    auto moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    start.z += moveDirection.z * speed * dt;

    e->SetStartPosition(start);

    pos.z = start.z;
    pos.x = start.x + sin(waveTime * waveFrequency) * waveAmplitude;

    e->SetPosition(pos);

    if (pos.z < ScissorsGameState::stageMinZ)
    {
        pos.z = ScissorsGameState::stageMinZ;
        start.z = pos.z;
        moveDirection.z = 1.0f;
    }
    else if (pos.z > ScissorsGameState::stageMaxZ)
    {
        pos.z = ScissorsGameState::stageMaxZ;
        start.z = pos.z;
        moveDirection.z = -1.0f;
    }


    e->SetMoveDirection(moveDirection);
    e->Face(moveDirection);
}

void ChaseBehavior::Update(EnemyBase* e, float dt)
{
    auto player = e->GetPlayer();
    if (!player) return;

    auto pos = e->GetPosition();
    auto target = player->GetPosition();

    // ÉvÉåÉCÉÑÅ[Ç÷ÇÃï˚å¸
    DirectX::XMFLOAT3 dir = MathHelper::Normalize(MathHelper::Subtract(target, pos));

    // ìGìØémÇ™èdÇ»ÇÁÇ»Ç¢ÇÊÇ§Ç…ï™ó£
    DirectX::XMFLOAT3 separation = { 0,0,0 };
    auto enemies = e->GetOwnerScene()->GetActorManager()->GetActorsOfType<EnemyBase>();

    for (auto& other : enemies)
    {
        if (other.get() == e) continue;

        auto otherPos = other->GetPosition();

        auto diff = MathHelper::Subtract(pos, otherPos);
        float distSq = diff.x * diff.x + diff.z * diff.z;

        if (distSq < avoidDist * avoidDist)
        {
            separation = MathHelper::Add(separation, diff);
        }
    }

    // çáê¨
    dir = MathHelper::Add(dir, MathHelper::Multiply(separation, separationWeight));
    dir = MathHelper::Normalize(dir);

    e->Move(dir, dt);
    e->Face(dir);
}

void RescueBehavior::Enter(EnemyBase* e)
{
    e->EnableScissorsVisual();
}

void RescueBehavior::Update(EnemyBase* e, float dt)
{
    // É^Å[ÉQÉbÉgÇ™Ç»Ç¢ or ñ≥å¯Ç»ÇÁíTÇ∑
    if (!target || target->IsDead() || target->GetState() != EnemyBase::YarnState::Tied)
    {
        target = FindTiedEnemy(e);
        if (!target) return;
    }

    auto pos = e->GetPosition();
    auto targetPos = target->GetPosition();

    XMFLOAT3 dir =
    {
        targetPos.x - pos.x,
        0,
        targetPos.z - pos.z
    };

    float len = sqrt(dir.x * dir.x + dir.z * dir.z);
    if (len < 0.01f) return;

    dir.x /= len;
    dir.z /= len;

    e->Move(dir, dt);
    e->Face(dir);

    // ãﬂÇ√Ç¢ÇΩÇÁã~èo
    if (len < 1.5f)
    {
        target->ReleasedTied();
        target = nullptr;
    }
}

EnemyBase* RescueBehavior::FindTiedEnemy(const EnemyBase* self)
{
    auto scene = self->GetOwnerScene();
    auto enemies = scene->GetActorManager()->GetActorsOfType<EnemyBase>();

    EnemyBase* nearest = nullptr;
    float minDist = 99999.0f;

    for (auto enemy : enemies)
    {
        if (enemy.get() == self) continue;
        if (enemy->GetState() != EnemyBase::YarnState::Tied) continue;

        float dist = MathHelper::Distance(self->GetPosition(), enemy->GetPosition());
        if (dist < minDist)
        {
            minDist = dist;
            nearest = enemy.get();
        }
    }

    return nearest;
}