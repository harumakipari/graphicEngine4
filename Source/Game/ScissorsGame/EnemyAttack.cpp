#include "pch.h"
#include "EnemyAttack.h"

#include "EnemyBase.h"
#include "NeedleActor.h"
#include "ScissorsGameState.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void NeedleAttack::Update(EnemyBase* e, float dt)
{
    stateTimer -= dt;

    switch (state)
    {
    case AttackState::Idle:
        UpdateWander(e, dt);

        if (stateTimer <= 0.0f)
        {
            basePos = e->GetPosition();
            state = AttackState::Charge;
            stateTimer = chargeTime; // 溜め時間
        }
        break;

    case AttackState::Charge:
        UpdateCharge(e, dt);

        if (stateTimer <= 0.0f)
        {
            state = AttackState::Fire;
            stateTimer = fireTime;
            FireRadial(e);
        }
        break;

    case AttackState::Fire:
        UpdateFire(e, dt);

        if (stateTimer <= 0.0f)
        {
            state = AttackState::Idle;
            stateTimer = interval;
        }
        break;
    }


}

// 溜める（縮む）
void NeedleAttack::UpdateCharge(EnemyBase* e, float dt)
{
    float t = 1.0f - (stateTimer / chargeTime);

    float scale = 1.0f - t * 0.3f; // 1 → 0.7
    e->SetScale({ scale, scale, scale });

    // 震え
    float shake = (1.0f - t) * 0.1f; // 終盤ほど強く
    float offsetX = MathHelper::RandomRange(-shake, shake);
    float offsetZ = MathHelper::RandomRange(-shake, shake);

    auto pos = basePos;
    pos.x += offsetX;
    pos.z += offsetZ;
    e->SetPosition(pos);
}

// 発射
void NeedleAttack::UpdateFire(EnemyBase* e, float dt)
{
    float t = 1.0f - (stateTimer / fireTime);

    float scale = 0.7f + t * 0.6f; // 0.7 → 1.3
    e->SetScale({ scale, scale, scale });
}

// 円状に放つ
void NeedleAttack::FireRadial(EnemyBase* e)
{
    bool isEight = useEight;   // 先に保存
    int count = isEight ? 8 : 4;
    useEight = !useEight;

    float offset = isEight ? 0.0f : DirectX::XM_PI / count;

    auto enemyPos = e->GetPosition();

    for (int i = 0; i < count; i++)
    {
        float angle = (DirectX::XM_2PI / count) * i + offset;

        XMFLOAT3 dir =
        {
            cosf(angle),
            0.0f,
            sinf(angle)
        };
        float distance = MathHelper::RandomRange(2.5f, 4.0f);
        XMFLOAT3 target =
        {
            enemyPos.x + dir.x * distance,
            0.0f,
            enemyPos.z + dir.z * distance
        };

        target.y = MathHelper::RandomRange(2.0f, 3.0f);

        target.x = std::clamp(
            enemyPos.x + dir.x * distance,
            ScissorsGameState::stageMinX,
            ScissorsGameState::stageMaxX
        );

        target.z = std::clamp(
            enemyPos.z + dir.z * distance,
            ScissorsGameState::stageMinZ,
            ScissorsGameState::stageMaxZ
        );

        target.y = 0.0f;

        Transform needleTr(enemyPos, { 0,0,0 }, { 1,1,1 });

        auto needle = e->GetOwnerScene()->GetActorManager()
            ->CreateAndRegisterActorWithTransform<NeedleActor>("needle", needleTr);

        needle->SetTargetPos(target);
    }
}

void NeedleAttack::UpdateWander(EnemyBase* e, float dt)
{
    auto pos = e->GetPosition();

    // ターゲットがない or 近づいたら更新
    float dist = MathHelper::Length(MathHelper::Subtract(wanderTarget, pos));
    if (dist < 1.0f)
    {
#if 0
        wanderTarget =
        {
            MathHelper::RandomRange(ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX),
            0.0f,
            MathHelper::RandomRange(ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ)
        };
#else
        wanderTarget =
        {
            MathHelper::RandomRange(e->rng, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX),
            0.0f,
            MathHelper::RandomRange(e->rng,ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX)
        };
#endif // 0
    }

    auto dir = MathHelper::Normalize(MathHelper::Subtract(wanderTarget, pos));

    // 向き
    e->Face(dir);

    // 移動
    float speed = e->GetSpeed();
    pos.x += dir.x * speed * dt;
    pos.z += dir.z * speed * dt;

    e->SetPosition(pos);

    // スケール戻す
    e->SetScale({ 1.0f,1.0f,1.0f });
}