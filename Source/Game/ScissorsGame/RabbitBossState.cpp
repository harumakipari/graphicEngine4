#include "pch.h"
#include "RabbitBossState.h"

#include "RabbitBossEnemy.h"


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
    int select = MathHelper::RandomRange(0, 1);

    if (select == 0)
    {// ワープ攻撃予兆へ遷移
        enemy->GetStateMachine()->ChangeState("WarpPreview");
    }
    else
    {// バフ攻撃へ遷移
        enemy->GetStateMachine()->ChangeState("BuffPreview");
    }
}

void RabbitBossAttackSelectState::Exit()
{

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

}

void RabbitBossAttackWarpState::Execute(float deltaTime)
{
    enemy->SpawnRandomPoint();

    enemy->GetStateMachine()->ChangeState("Idle");
}

void RabbitBossAttackWarpState::Exit()
{

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

}

