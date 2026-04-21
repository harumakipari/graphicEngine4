#include "pch.h"
#include "ScissorsPlayerStateDerived.h"
#include "Game/Actors/Base/Character.h"
#include "ScissorsPlayer1.h"
#include "Physics/CollisionFunction.h"

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
    player->footstepAudioComponent->Play();
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
    player->footstepAudioComponent->Stop();
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

    // ハサミ攻撃を一体の敵のみに当てるため。
    player->hasDamageEnemy = false;
    player->debugScissorsCollisionColor = { 1,0,0,1.0f }; // デバッグ用にハサミ攻撃の当たり判定の色を変える　通常は白色で、ダメージを受けたときに赤くするなどして使用する
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

    player->debugScissorsCollisionColor = { 1,1,1,0.0f }; // デバッグ用にハサミ攻撃の当たり判定の色を変える　通常は白色で、ダメージを受けたときに赤くするなどして使用する
}

void ScissorsPlayerChargeDashState::Enter()
{
    // ダッシュアニメーションを再生
    player->PlayAnimation("ChargeDash", false, true, 0.1f);
    // ダッシュの狙いを表示する矢印のUIコンポーネントを表示する
    player->dashAimArrowComponent->SetVisible(true);
    // チャージ音を再生する
    //player->chargeAudioComponent->Play();

    // スタンするかどうかフラグをリセットする
    player->isStun = false;

    player->debugDashCollisionColor = { 1,0,0,1 }; // デバッグ用にダッシュの当たり判定の色を変える　通常は透明で、攻撃中は赤くするなどして使用する
}

void ScissorsPlayerChargeDashState::Execute(float deltaTime)
{
    // ダッシュの方向や溜めの強さを取得する
    auto aimData = player->GetAimData();
    // ダッシュの方向
    DirectX::XMFLOAT3 dashDir = aimData.dir;

    //if (!InputSystem::IsGamepadConnected())
    //{
    //    player->rotationComponent->SetDirection(dashDir);
    //}

    float aimDashPower = aimData.power;
    float dashDistance = minDistance + aimData.power * (maxDistance - minDistance);;
    //float dashDistance = 10.0f;

    // ダッシュの移動先を計算する　
    DirectX::XMFLOAT3 pos = player->GetPosition();
    DirectX::XMFLOAT3 unclampedTarget = { pos.x + dashDir.x * dashDistance,pos.y + dashDir.y * dashDistance,pos.z + dashDir.z * dashDistance };



    DirectX::XMFLOAT3 clampedTarget = unclampedTarget;
    // ステージ外に出ないようにクランプ
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;
    clampedTarget.x = std::clamp(clampedTarget.x, stageMinX, stageMaxX);
    clampedTarget.z = std::clamp(clampedTarget.z, stageMinZ, stageMaxZ);

    HitResultWithActor hitResult = {};
    uint32_t mask = CollisionHelper::ToBit(CollisionLayer::RibbonWall);
    if (CollisionFunction::SphereRayCast(pos, clampedTarget, hitResult, 0.1f, mask))
    {// もしリボンの壁に当たっていたら
        clampedTarget.x = hitResult.hitPoint.x;
        clampedTarget.z = hitResult.hitPoint.z;
    }

    // 差があるかチェック
    player->isStun =
        (unclampedTarget.x != clampedTarget.x) ||
        (unclampedTarget.z != clampedTarget.z);

    player->targetPos = clampedTarget;

    // 目的地のスクリーン座標
    XMFLOAT2 uiTargetPos = WorldToUI(player->targetPos);
    // プレイヤーの位置のスクリーン座標
    XMFLOAT2 uiPlayerPos = WorldToUI(pos);

    float distance = MathHelper::DistanceFloat2(uiTargetPos, uiPlayerPos);

    float arrowSizeX = player->dashAimArrowComponent->GetSize().x;
    float uiScale = abs(distance) / arrowSizeX;
    player->dashAimArrowComponent->SetScale({ uiScale,1.0f });

    //　方向ベクトル
    DirectX::XMFLOAT2 dir = MathHelper::SubtractFloat2(uiTargetPos, uiPlayerPos);
    float angle = atan2f(dir.y, dir.x);
    player->dashAimArrowComponent->SetWorldAngleDegree(DirectX::XMConvertToDegrees(angle));


    //float angle = DirectX::XMConvertToDegrees(atan2f(aimData.dir.x, aimData.dir.z));


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

    // チャージ音を止める
    player->chargeAudioComponent->Stop();

    // この次は絶対にダッシュステートに行くので、ここでは何もしない

}

void ScissorsPlayerDashState::Enter()
{
    // ダッシュアニメーションを再生
    player->PlayAnimation("Dash", false, true, 0.1f);

    startPos = player->GetPosition();
    elapsedTime = 0.0f;

    // ダッシュ音を再生する
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/dash.wav", 0.5f);
}

void ScissorsPlayerDashState::Execute(float deltaTime)
{
    // ヒットストップ中は何もしない
    if (player->hitStopTimer > 0.0f)
    {
        player->hitStopTimer -= deltaTime;
        return;
    }

    // 軌跡地点を追加
    XMFLOAT3 playerPos = player->GetPosition();
    XMFLOAT3 trailPosition = playerPos;
    trailPosition.y += 0.4f; // 床に被るの防ぐために浮かせる
    player->trail.trailPoints.push_back({ trailPosition, 0.5f });
    DirectX::XMFLOAT3 dir = player->GetForward();

#if 0
    // 星のエフェクトを出す
    player->SpawnStarParticle(playerPos, dir);
#endif // 0

    elapsedTime += deltaTime;

    // 時間固定ダッシュ 何秒でtargetPosに着くか
    float t = elapsedTime / dashDuration;
    t = std::clamp(t, 0.0f, 1.0f);

    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;

    // 補間
    DirectX::XMFLOAT3 nextPos = MathHelper::Lerp(startPos, player->targetPos, t);
    // 壁チェック（簡易版）
    if (nextPos.x <= stageMinX || nextPos.x >= stageMaxX ||
        nextPos.z <= stageMinZ || nextPos.z >= stageMaxZ)
    {
        player->GetStateMachine()->ChangeState("Stun");
        return;
    }

    player->SetPosition(nextPos);

    if (t >= 1.0f)
    {
        player->GetStateMachine()->ChangeState("Idle");
    }

}

void ScissorsPlayerDashState::Exit()
{
    player->PlayAnimation("Idle", true, true, 0.1f);
    player->characterMovementComponent->ResetSpeed(); // ダッシュが終わったら移動速度をリセットする

    player->debugDashCollisionColor = { 1,1,1,0 }; // デバッグ用にダッシュの当たり判定の色を変える　通常は透明で、攻撃中は赤くするなどして使用する

}


void ScissorsPlayerStunState::Enter()
{
    stunTimer = 0.0f;

    player->PlayAnimation("Idle", true, true);
    player->characterMovementComponent->SetSpeed(0.0f);

    // ここでカメラシェイクやSEなどを入れる

}

void ScissorsPlayerStunState::Execute(float deltaTime)
{
    stunTimer += deltaTime;

    if (stunTimer > stunDuration)
    {
        player->GetStateMachine()->ChangeState("Idle");
    }
}

void ScissorsPlayerStunState::Exit()
{
    player->characterMovementComponent->ResetSpeed();
    player->isStun = false;
}
