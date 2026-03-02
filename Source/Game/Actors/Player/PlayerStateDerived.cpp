#include "pch.h"
#include "PlayerStateDerived.h"
#include "Game/Actors/Base/Character.h"
#include "Game/Actors/Player/Player.h"

PlayerStateBase::PlayerStateBase(Player* actor) :State(actor), player(actor)
{
}

void PlayerIdleState::Enter()
{
    owner->PlayAnimation("Idle");
}

void PlayerIdleState::Execute(float deltaTime)
{
    // 攻撃入力チェック
    if (InputSystem::GetInputState("Attack", InputStateMask::Trigger))
    {
        player->GetStateMachine()->ChangeState("Attack");
        return;
    }

    // 入力があれば走るステートに変更
    auto inputComp = player->inputComponent;
    DirectX::XMFLOAT3 dir = inputComp->GetMoveInput();

    if (std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON)
    {
        return;
    }
    player->GetStateMachine()->ChangeState("Running");
}

void PlayerIdleState::Exit()
{

}

void PlayerRunningState::Enter()
{
    owner->PlayAnimation("Jog_Fwd", true, true, 0.2f);
}

void PlayerRunningState::Execute(float deltaTime)
{
    // 攻撃入力チェック
    if (InputSystem::GetInputState("Attack", InputStateMask::Trigger))
    {
        player->GetStateMachine()->ChangeState("Attack");
        return;
    }

    // 入力がなければ待機ステートに変更
    auto inputComp = player->inputComponent;
    DirectX::XMFLOAT3 dir = inputComp->GetMoveInput();
    if (std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON)
    {
        player->GetStateMachine()->ChangeState("Idle");
        return;
    }
}

void PlayerRunningState::Exit()
{
}

void PlayerAttackingState::Enter()
{
    owner->PlayAnimation("Primary_Attack_Fast_A", false, true, 0.1f);
}

void PlayerAttackingState::Execute(float deltaTime)
{
    // アニメーションが終わったら待機ステートに変更
    if (!owner->GetAnimationController()->IsPlayAnimation())
    {
        player->GetStateMachine()->ChangeState("Idle");
    }
}

void PlayerAttackingState::Exit()
{
}
