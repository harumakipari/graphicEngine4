#include "pch.h"
#include "ScissorsPlayerStateDerived.h"

#include "BobbinActor.h"
#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
#include "ScissorsGameEnemyBaseActor.h"
#include "Game/Actors/Base/Character.h"
#include "ScissorsPlayer1.h"
#include "YarnEnemyActor.h"
#include "Engine/Utility/Time.h"
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
    // チャージダッシュアニメーションを再生
    player->PlayAnimation("ChargeDash", false, true, 0.1f);

    // アニメーションの速度を速くする
    player->GetAnimationController()->SetAnimationRate(2.0f); 

    // チャージ音を再生する
    //player->chargeAudioComponent->Play();

    // スタンするかどうかフラグをリセットする
    player->isStun = false;

    // プレイヤーのダッシュの位置の保存を削除する
    player->dashPoints.clear();

    // ダッシュ中に倒した敵をリセットする
    player->killedEnemyCountInDash = 0;

    player->debugDashCollisionColor = { 1,0,0,1 }; // デバッグ用にダッシュの当たり判定の色を変える　通常は透明で、攻撃中は赤くするなどして使用する
}

void ScissorsPlayerChargeDashState::Execute(float deltaTime)
{
    // ダッシュの方向や溜めの強さを取得する
    auto aimData = player->GetAimData();
    // ダッシュの方向
    dashDir = aimData.dir;
    // 今の位置
    currentPos = player->GetPosition();

    // プレイヤーのダッシュの位置の保存を削除する
    player->dashPoints.clear();
    player->dashPoints.push_back(currentPos); // 始点を入れる

    /*if (!InputSystem::IsGamepadConnected())
    {
        player->rotationComponent->SetDirection(dashDir);
    }*/

    float dashDistance = minDistance + aimData.power * (maxDistance - minDistance);
    float remainingDist = dashDistance;

    // ダッシュの距離
    for (int i = 0; i < 5; i++) // 最大5回反射
    {
        XMFLOAT3 nextTarget =
        {
            currentPos.x + dashDir.x * remainingDist,
            currentPos.y,
            currentPos.z + dashDir.z * remainingDist
        };

        HitResultWithActor hit;
        uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Wall) | CollisionHelper::ToBit(CollisionLayer::EnemyRedirect)
            | CollisionHelper::ToBit(CollisionLayer::Boss) | CollisionHelper::ToBit(CollisionLayer::Bobbin);

        if (CollisionFunction::SphereRayCast(currentPos, nextTarget, hit, 0.2f, mask))
        {
            // 壁に当たった地点
            player->dashPoints.push_back(hit.hitPoint);

#if 1
            if (auto boss = dynamic_cast<RabbitBossEnemyActor*>(hit.actor))
            {// ボスに当たったら、
                //Logger::Log(U8("ダッシュ予測中にボスに当たった"));
                break;
            }
            if (auto bobbin = dynamic_cast<BobbinActor*>(hit.actor))
            {// ボスに当たったら、
                //Logger::Log(U8("ダッシュ予測中にボスに当たった"));
                break;
            }
#endif
            // 残り距離
            float traveled = MathHelper::Distance(currentPos, hit.hitPoint);
            traveled = std::min<float>(traveled, remainingDist);

            //remainingDist -= traveled;
            remainingDist = dashDistance * 0.8f;

            if (remainingDist < 0.01f)
            {
                player->dashPoints.push_back(hit.hitPoint);
                break;
            }


            // 反射敵にヒットした場合
            if (auto enemy = dynamic_cast<EnemyBase*>(hit.actor))
            {
                // 敵ヒット → 90度曲がり
                // 敵の中心
                XMFLOAT3 enemyPos = enemy->GetPosition();

                // ヒット位置との差
                XMFLOAT3 local =
                {
                    hit.hitPoint.x - enemyPos.x,
                    0.0f,
                    hit.hitPoint.z - enemyPos.z
                };

                // 敵の右方向（なければ {1,0,0}）
                XMFLOAT3 right = { 1,0,0 };
                //XMFLOAT3 right = enemy->GetRight();

                // dotで左右判定
                float dot = local.x * right.x + local.z * right.z;

                // 90度回転
                if (dot > 0)
                {
                    // 右側ヒット → 右に曲がる
                    dashDir = { dashDir.z, 0.0f, -dashDir.x };
                }
                else
                {
                    // 左側ヒット → 左に曲がる
                    dashDir = { -dashDir.z, 0.0f, dashDir.x };
                }
                //  正規化
                float len = sqrt(dashDir.x * dashDir.x + dashDir.z * dashDir.z);
                if (len > 0.0001f)
                {
                    dashDir.x /= len;
                    dashDir.z /= len;
                }

                //  進行方向に押し出す（ここ重要）
                const float pushOut = 1.0f;

                currentPos =
                {
                    hit.hitPoint.x + dashDir.x * pushOut,
                    hit.hitPoint.y,
                    hit.hitPoint.z + dashDir.z * pushOut
                };
            }
            else
            {
                // 反射
                XMFLOAT3 normal = hit.normal;


                float dot = dashDir.x * normal.x + dashDir.z * normal.z;

                dashDir.x = dashDir.x - 2 * dot * normal.x;
                dashDir.z = dashDir.z - 2 * dot * normal.z;


                // 正規化
                float len = sqrt(dashDir.x * dashDir.x + dashDir.z * dashDir.z);
                if (len < 0.0001f)
                {
                    // fallback（元の方向維持）
                    dashDir = aimData.dir;
                }
                else
                {
                    dashDir.x /= len;
                    dashDir.z /= len;
                }
                currentPos = hit.hitPoint;

                // 反射後 壁に埋まっているのを直す
                const float pushOut = 1.0f;

                currentPos =
                {
                    hit.hitPoint.x + normal.x * pushOut,
                    hit.hitPoint.y,
                    hit.hitPoint.z + normal.z * pushOut
                };
            }
            DebugRender::DrawLine(hit.hitPoint,
                { hit.hitPoint.x + hit.normal.x,
                  hit.hitPoint.y,
                  hit.hitPoint.z + hit.normal.z },
                { 1,1,0,1 });

        }
        else
        {
            player->dashPoints.push_back(nextTarget);
            break;
        }
    }

    int arrowCount = _countof(player->arrowComponents);
    int segmentCount = static_cast<int>(player->dashPoints.size()) - 1;

    int count = std::min<int>(arrowCount, segmentCount);

    // 全部非表示
    for (int i = 0; i < arrowCount; i++)
    {
        player->arrowComponents[i]->SetVisible(false);
    }

    // 必要な分だけ表示
    for (int i = 0; i < count; i++)
    {
        player->arrowComponents[i]->SetStart(player->dashPoints[i]);
        player->arrowComponents[i]->SetEnd(player->dashPoints[i + 1]);
        player->arrowComponents[i]->SetVisible(true);
    }


    if (player->IsDashTriggered())
    {
        player->fixedDashPoints = player->dashPoints;


        player->UseDash();
        // ダッシュトリガーが離されたらダッシュステートに移行する
        player->GetStateMachine()->ChangeState("Dash");
    }
}

void ScissorsPlayerChargeDashState::Exit()
{
    // ダッシュの狙いを表示する矢印のUIコンポーネントを非表示にする
    for (int i = 0; i < _countof(player->arrowComponents); i++)
    {
        player->arrowComponents[i]->SetVisible(false);
    }

    // チャージ音を止める
    player->chargeAudioComponent->Stop();

    // アニメーションの速度を元に戻す
    player->GetAnimationController()->SetAnimationRate(1.0f);

    // この次は絶対にダッシュステートに行くので、ここでは何もしない

}

void ScissorsPlayerDashState::Enter()
{
    // ダッシュアニメーションを再生
    player->PlayAnimation("Dash", true, true, 0.2f);

    startPos = player->GetPosition();
    elapsedTime = 0.0f;


    if (player->fixedDashPoints.size() < 2)
    {
        player->GetStateMachine()->ChangeState("Idle");
        return;
    }

    segmentStart = player->fixedDashPoints[0];
    segmentEnd = player->fixedDashPoints[1];

    float dist = MathHelper::Distance(segmentStart, segmentEnd);
    segmentDuration = dist / speed;
    segmentElapsed = 0.0f;

    // プレイヤーのダッシュの値をリセット
    player->currentSegment = 0;

    // 全敵のヒット情報リセット
    auto enemies = player->GetOwnerScene()->GetActorManager()->GetActorsOfType<EnemyBase>();
    for (auto e : enemies)
    {
        e->lastHitSegment = -1;
    }

    // ダッシュ音を再生する
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/dash.wav", 0.5f);

    // 当たり判定の押出を消す
    player->sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::None);
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
    XMFLOAT3 currentPos = player->GetPosition();
    XMFLOAT3 trailPosition = currentPos;
    trailPosition.y += 0.4f; // 床に被るの防ぐために浮かせる
    player->trail.trailPoints.push_back({ trailPosition, 1.5f });

#if 0
    // 星のエフェクトを出す
    player->SpawnStarParticle(currentPos, dir);
#endif // 0
    elapsedTime += deltaTime;
    segmentElapsed += deltaTime;

    float t = segmentElapsed / segmentDuration;
    t = std::clamp(t, 0.0f, 1.0f);

    // 補間
    XMFLOAT3 nextPos = MathHelper::Lerp(segmentStart, segmentEnd, t);
    player->SetPosition(nextPos);

#if 1
    // ボスと糸巻がいるかどうかの判定 
    {
        XMFLOAT3 prevPos = currentPos;
        HitResultWithActor hit;
        uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Boss) | CollisionHelper::ToBit(CollisionLayer::Bobbin);

        if (CollisionFunction::SphereRayCast(prevPos, nextPos, hit, 0.5f, mask))
        {
            // ヒット位置に補正
            player->SetPosition({ hit.hitPoint.x,0.0f,hit.hitPoint.z });

            // ダッシュ停止
            player->GetStateMachine()->ChangeState("Idle");
            return;
        }
    }


#endif // 0
#if 1
    // 敵がいるかどうかの判定
    {
        XMFLOAT3 prevPos = currentPos;
        HitResultWithActor hit;
        uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Enemy);

        float radius = 1.5f;
        if (CollisionFunction::SphereRayCast(prevPos, nextPos, hit, radius, mask))
        {
            if (auto enemy = dynamic_cast<EnemyBase*>(hit.actor))
            {
                // 敵に吸着
                nextPos = enemy->GetPosition();
            }
        }
    }

#endif // 1



    DirectX::XMFLOAT3 playerDir = MathHelper::Normalize(MathHelper::Subtract(nextPos, currentPos));
    player->rotationComponent->SetDirection(playerDir);

    // セグメント終了
    if (t >= 1.0f)
    {
        player->currentSegment++;

        if (player->currentSegment >= player->fixedDashPoints.size() - 1)
        {
            player->GetStateMachine()->ChangeState("Idle");
            return;
        }

#if 0
        // 曲がりでスロー再生
        if (player->currentSegment > 0)
        {
            Time::SetSlow(0.6f, 0.06f);
        }
#endif // 0

        // 次の区間へ
        segmentStart = player->fixedDashPoints[player->currentSegment];
        segmentEnd = player->fixedDashPoints[player->currentSegment + 1];

        float dist = MathHelper::Distance(segmentStart, segmentEnd);
        segmentDuration = dist / 20.0f;
        segmentElapsed = 0.0f;

        // 向き更新
        XMFLOAT3 dir =
        {
            segmentEnd.x - segmentStart.x,
            0,
            segmentEnd.z - segmentStart.z
        };

        float len = sqrt(dir.x * dir.x + dir.z * dir.z);
        if (len > 0.0001f)
        {
            dir.x /= len;
            dir.z /= len;
            player->rotationComponent->SetDirection(dir);
        }
    }

#if 0
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
#endif // 0


}

void ScissorsPlayerDashState::Exit()
{
    player->PlayAnimation("Idle", true, true, 0.5f);
    player->characterMovementComponent->ResetSpeed(); // ダッシュが終わったら移動速度をリセットする

    player->debugDashCollisionColor = { 1,1,1,0 }; // デバッグ用にダッシュの当たり判定の色を変える　通常は透明で、攻撃中は赤くするなどして使用する

    // 当たり判定の押出を戻す
    player->sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);

    // 反射敵の通知
    player->ResolveReflectedKills();

    // ダッシュ後の無敵時間を設定する
    player->postDashInvincibleTimer = player->postDashInvincibleDuration;

    // ダッシュ中に倒した敵の数をリセットする
    player->killedEnemyCountInDash = 0;

}


void ScissorsPlayerStunState::Enter()
{
    stunTimer = 0.0f;

    player->PlayAnimation("Idle", true, true, 0.5f);
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
