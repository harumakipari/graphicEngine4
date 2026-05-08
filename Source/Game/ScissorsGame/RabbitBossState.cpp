#include "pch.h"
#include "RabbitBossState.h"

#include "BossSpawner.h"
#include "RabbitBossEnemy.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"


RabbitBossStateBase::RabbitBossStateBase(RabbitBossEnemyActor* enemy) :State(enemy), enemy(enemy)
{
}

void RabbitBossIdleState::Enter()
{
    attackTimer = attackTimerInterval;
}

void RabbitBossIdleState::Execute(float deltaTime)
{
    attackTimer -= deltaTime;
    if (attackTimer < 0.0f)
    {// 攻撃選択ステートへ遷移する
        enemy->GetStateMachine()->ChangeState("AttackSelect");
    }
}

void RabbitBossIdleState::Exit()
{

}

// 攻撃選択
void RabbitBossAttackSelectState::Enter()
{

}

void RabbitBossAttackSelectState::Execute(float deltaTime)
{
    BossAttackType type = PopAttack();

    switch (type)
    {
    case BossAttackType::Warp:
        enemy->GetStateMachine()->ChangeState("WarpPreview");
        break;

    case BossAttackType::Buff:
        enemy->GetStateMachine()->ChangeState("BuffPreview");
        break;
    }
}

void RabbitBossAttackSelectState::Exit()
{

}

// 攻撃を選ぶバッグを生成する
void RabbitBossAttackSelectState::RefillAttackBag()
{
    attackBag.clear();

    // Warp 4個
    for (int i = 0; i < 4; i++)
    {
        attackBag.push_back(BossAttackType::Warp);
    }

    // Buff 1個
    attackBag.push_back(BossAttackType::Buff);

    std::shuffle(attackBag.begin(), attackBag.end(), rng);
}

// 攻撃タイプを取り出す
RabbitBossAttackSelectState::BossAttackType RabbitBossAttackSelectState::PopAttack()
{
    if (attackBag.empty())
    {
        RefillAttackBag();
    }

    BossAttackType type = attackBag.back();
    attackBag.pop_back();

    return type;
}


// ワーププレビュー
void RabbitBossAttackWarpPreviewState::Enter()
{
}

void RabbitBossAttackWarpPreviewState::Execute(float deltaTime)
{
    enemy->GetStateMachine()->ChangeState("Warp");
}

void RabbitBossAttackWarpPreviewState::Exit()
{

}

// ワープ
void RabbitBossAttackWarpState::Enter()
{
    phase = WarpPhase::Dive;
    timer = 0.0f;
    enemy->StartDive();
    // 当たり判定を無効にする
    enemy->collisionBoxComponent->DisableCollision();
}

void RabbitBossAttackWarpState::Execute(float deltaTime)
{
    switch (phase)
    {
    case WarpPhase::Dive:
        if (enemy->IsFinishedDive())
        {// 潜りが終わったら
            phase = WarpPhase::Chase;
        }
        break;
    case WarpPhase::Chase:
        timer += deltaTime;

        if (auto player = enemy->GetPlayer())
        {
            DirectX::XMFLOAT3 playerPos = player->GetPosition();
            DirectX::XMFLOAT3 pos = enemy->GetPosition();

            DirectX::XMFLOAT3 dir;
            dir.x = playerPos.x - pos.x;
            dir.z = playerPos.z - pos.z;
            dir.y = 0.0f;

            dir = MathHelper::Normalize(dir);

            float moveSpeed = 5.0f;

            pos.x += dir.x * moveSpeed * deltaTime;
            pos.z += dir.z * moveSpeed * deltaTime;


            enemy->SetPosition(pos);
        }
        if (timer > chaseTime)
        {
            phase = WarpPhase::Emerge;
            timer = 0.0f;
            enemy->StartEmerge();
        }
        break;
    case WarpPhase::Emerge:
        if (enemy->IsFinishedEmerge())
        {
            // 出現ダメージ
           enemy->ApplyLandingDamage(); 

            enemy->GetStateMachine()->ChangeState("Idle");
        }
        break;
    }

}

void RabbitBossAttackWarpState::Exit()
{
    // 当たり判定を有効にする
    enemy->collisionBoxComponent->EnableCollision();
}

// バフプレビュー
void RabbitBossAttackBuffPreviewState::Enter()
{

}

void RabbitBossAttackBuffPreviewState::Execute(float deltaTime)
{
    enemy->GetStateMachine()->ChangeState("Buff");
}

void RabbitBossAttackBuffPreviewState::Exit()
{

}

// バフ
void RabbitBossAttackBuffState::Enter()
{

}

void RabbitBossAttackBuffState::Execute(float deltaTime)
{
    enemy->EnlargeRandomEnemies(enemyBuffCount);

    enemy->GetStateMachine()->ChangeState("Idle");
}

void RabbitBossAttackBuffState::Exit()
{

}

// スタン
void RabbitBossStunState::Enter()
{
    stunTimer = stunTimerInterval;

    // スタンのモデルの見た目をオンにする
    enemy->stunModel->SetIsVisible(true);
    // 当たり判定を有効にする
    enemy->collisionBoxComponent->EnableCollision();
}

void RabbitBossStunState::Execute(float deltaTime)
{
    stunTimer -= deltaTime;
    if (stunTimer < 0.0f)
    {// 待機ステートへ遷移する
        enemy->GetStateMachine()->ChangeState("Idle");
    }
}

void RabbitBossStunState::Exit()
{
    enemy->stunModel->SetIsVisible(false);
}


// 死亡
void RabbitBossDeathState::Enter()
{
    enemy->collisionBoxComponent->DisableCollision();
    elapsedTime = 0.0f;
    // ボスが死亡したら呼ぶ処理  一フレームのみ
    enemy->StartDeathPerform();

    // 敵の出現を終了させる
    if (auto bossSpawner=enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }
}

void RabbitBossDeathState::Execute(float deltaTime)
{
    enemy->UpdateDead(deltaTime);

    elapsedTime += deltaTime;

    if (elapsedTime >= 5.0f)
    {
        enemy->EndDeathPerform();
    }
}

void RabbitBossDeathState::Exit()
{
}

