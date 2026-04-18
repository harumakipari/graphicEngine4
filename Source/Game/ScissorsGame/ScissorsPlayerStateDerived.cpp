#include "pch.h"
#include "ScissorsPlayerStateDerived.h"
#include "Game/Actors/Base/Character.h"
#include "ScissorsPlayer1.h"

ScissorsPlayerStateBase::ScissorsPlayerStateBase(ScissorsPlayer1* actor) :State(actor), player(actor)
{
}

void ScissorsPlayerIdleState::Enter()
{
    owner->PlayAnimation("Idle");
}

void ScissorsPlayerIdleState::Execute(float deltaTime)
{
    // 攻撃入力チェック
    if (player->IsAttackTriggered())
    {
        player->GetStateMachine()->ChangeState("Attack");
    }

    // ダッシュ入力チェック
    if (player->IsChargeDashTriggered())
    {
        if (player->CanDash())
        {
            player->GetStateMachine()->ChangeState("ChargeDash");
        }
        else
        {
            player->FailDash();
        }
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

void ScissorsPlayerIdleState::Exit()
{

}

void ScissorsPlayerRunningState::Enter()
{
    owner->PlayAnimation("Run", true, true, 0.2f);
}

void ScissorsPlayerRunningState::Execute(float deltaTime)
{
    // 移動方向を取得して移動する 回転もする
    DirectX::XMFLOAT3 moveDir = player->GetMoveDirection();
    player->characterMovementComponent->SetMoveDirection(moveDir);

    // 攻撃入力チェック
    if (player->IsAttackTriggered())
    {
        player->GetStateMachine()->ChangeState("Attack");
    }

    // ダッシュ入力チェック
    if (player->IsChargeDashTriggered())
    {
        if (player->CanDash())
        {
            player->GetStateMachine()->ChangeState("ChargeDash");
        }
        else
        {
            player->FailDash();
        }
    }

    // 入力がなければ待機ステートに変更
    auto inputComp = player->inputComponent;
    DirectX::XMFLOAT3 dir = inputComp->GetMoveInput();
    if (std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON)
    {
        player->GetStateMachine()->ChangeState("Idle");
    }
}

void ScissorsPlayerRunningState::Exit()
{
    player->characterMovementComponent->SetMoveDirection({ 0,0,0 }); // 待機状態に入ったら移動方向を0にする
}

void ScissorsPlayerAttackingState::Enter()
{
    // 攻撃中は移動速度を0にする
    player->characterMovementComponent->SetSpeed(0.0f);

    // 攻撃アニメーションを再生
    player->PlayAnimation("Attack", false, true, 0.1f);

    // 攻撃ヒット処理
    player->DoAttackHit();

    // 攻撃タイマーをリセット
    attackTimer = 0.0f;
    hitDone = false;
}

void ScissorsPlayerAttackingState::Execute(float deltaTime)
{
#if 0
    attackTimer += deltaTime;

    // 0.3秒後に当たる
    if (!hitDone && attackTimer > 0.3f)
    {
        player->DoAttackHit();
        hitDone = true;
    }

#endif // 0

    if (!owner->GetAnimationController()->IsPlayAnimation())
    {// アニメーション終わったら戻る
        auto dir = player->inputComponent->GetMoveInput();
        if (MathHelper::Length(dir) > 0.01f)
        {
            player->GetStateMachine()->ChangeState("Running");
        }
        else
        {
            player->GetStateMachine()->ChangeState("Idle");
        }
    }

}

void ScissorsPlayerAttackingState::Exit()
{
    player->characterMovementComponent->ResetSpeed(); // 攻撃が終わったら移動速度をリセットする
}

void ScissorsPlayerChargeDashState::Enter()
{
    // ダッシュアニメーションを再生
    player->PlayAnimation("ChargeDash", false, true, 0.1f);
    // ダッシュの狙いを表示する矢印のUIコンポーネントを表示する
    player->dashAimArrowComponent->SetVisible(true);
}

void ScissorsPlayerChargeDashState::Execute(float deltaTime)
{
    // ダッシュの方向や溜めの強さを取得する
    auto aimData = player->GetAimData();
    // ダッシュの方向
    DirectX::XMFLOAT3 dashDir = aimData.dir;

    float aimDashPower = aimData.power;
    float dashDistance = minDistance + aimData.power * (maxDistance - minDistance);;
    //float dashDistance = 10.0f;

    // ダッシュの移動先を計算する　
    DirectX::XMFLOAT3 pos = player->GetPosition();
    player->targetPos = { pos.x + dashDir.x * dashDistance,pos.y + dashDir.y * dashDistance,pos.z + dashDir.z * dashDistance };

    // ステージ外に出ないようにクランプ
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;
    player->targetPos.x = std::clamp(player->targetPos.x, stageMinX, stageMaxX);
    player->targetPos.z = std::clamp(player->targetPos.z, stageMinZ, stageMaxZ);

    float distance = sqrt(player->targetPos.x * player->targetPos.x + player->targetPos.z * player->targetPos.z);
    float uiScale = 0.1f; // ←ここ調整ポイント
    float uiLength = std::abs(distance * uiScale);
    player->dashAimArrowComponent->SetScale({ 1.0f, uiLength });

    // ダッシュの方向にUIを出す
    DebugRender::DrawSphere(player->targetPos, 0.3f, { 1,0,0,1 });

    if (player->IsDashTriggered())
    {
        player->UseDash();
        // ダッシュトリガーが離されたらダッシュステートに移行する
        player->GetStateMachine()->ChangeState("Dash");
    }
}

void ScissorsPlayerChargeDashState::Exit()
{
    // ダッシュの狙いを表示する矢印のUIコンポーネントを非表示にする
    player->dashAimArrowComponent->SetVisible(false);



    // この次は絶対にダッシュステートに行くので、ここでは何もしない

}

void ScissorsPlayerDashState::Enter()
{
    // ダッシュアニメーションを再生
    player->PlayAnimation("Dash", false, true, 0.1f);

    startPos = player->GetPosition();
    elapsedTime = 0.0f;


}

void ScissorsPlayerDashState::Execute(float deltaTime)
{
    // ヒットストップ中は何もしない
    if (player->hitStopTimer > 0.0f)
    {
        player->hitStopTimer -= deltaTime;
        return;
    }

    elapsedTime += deltaTime;

    // 時間固定ダッシュ 何秒でtargetPosに着くか
    float t = elapsedTime / dashDuration;
    t = std::clamp(t, 0.0f, 1.0f);

    // 補間
    DirectX::XMFLOAT3 pos = MathHelper::Lerp(startPos, player->targetPos, t);
    player->SetPosition(pos);

    if (t >= 1.0f)
    {
        player->GetStateMachine()->ChangeState("Idle");
    }

}

void ScissorsPlayerDashState::Exit()
{
    player->PlayAnimation("Idle", true, true, 0.1f);
    player->characterMovementComponent->ResetSpeed(); // ダッシュが終わったら移動速度をリセットする
}

