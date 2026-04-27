#include "pch.h"
#include "EnemyAttack.h"

#include "EnemyBase.h"
#include "NeedleActor.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void NeedleAttack::Update(EnemyBase* e, float dt)
{
    cooldownTimer -= dt;

    if (cooldownTimer<0.0f)
    {
        Fire(e);
        cooldownTimer = interval;
    }
}


void NeedleAttack::Fire(EnemyBase* e)
{
    auto player = e->GetPlayer();
    if (!player) return;
    auto playerPosition = player->GetPosition();
    DirectX::XMFLOAT3 enemyPos = e->GetPosition();
    auto toPlayer = MathHelper::Subtract(playerPosition, enemyPos);
    auto dir = MathHelper::Normalize(toPlayer);

    // ‹——£ƒ‰ƒ“ƒ_ƒ€
    float dist = MathHelper::RandomRange(3.0f, 8.0f);

    // ‰¡ƒuƒŒ
    float side = MathHelper::RandomRange(-2.0f, 2.0f);

    XMFLOAT3 target =
    {
        enemyPos.x + dir.x * dist - dir.z * side,
        0.0f,
        enemyPos.z + dir.z * dist + dir.x * side
    };

    Transform needleTr(enemyPos, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto needleActor = e->GetOwnerScene()->GetActorManager()
        ->CreateAndRegisterActorWithTransform<NeedleActor>("needle",needleTr);
    needleActor->SetTargetPos(target);
}
