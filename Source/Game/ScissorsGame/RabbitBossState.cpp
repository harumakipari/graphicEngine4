#include "pch.h"
#include "RabbitBossState.h"

#include "BobbinActor.h"
#include "BossSpawner.h"
#include "RabbitBossEnemy.h"
#include "ScissorsGameState.h"
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
    //type = BossAttackType::Buff;
#if 0
    switch (type)
    {
    case BossAttackType::Warp:
        enemy->GetStateMachine()->ChangeState("WarpPreview");
        break;

    case BossAttackType::Buff:
        enemy->GetStateMachine()->ChangeState("BuffPreview");
        break;
    }
#endif // 0
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
    enemy->PlayAnimation("WarpStart", false, true, 0.1f);
    elapsedTime = 0.0f;
}

void RabbitBossAttackWarpPreviewState::Execute(float deltaTime)
{
    elapsedTime += deltaTime;
    if (elapsedTime > 0.9f)
    {
        enemy->GetStateMachine()->ChangeState("Warp");
    }
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

    // 最初は追尾マークのみ見た目を有効にする
    enemy->bossSpawnMarkModel->SetIsVisible(false);

}

void RabbitBossAttackWarpState::Execute(float deltaTime)
{
    DirectX::XMFLOAT3 pos = enemy->GetPosition();
    timer += deltaTime;

    switch (phase)
    {
    case WarpPhase::Dive:
        if (enemy->IsFinishedDive())
        {// 潜りが終わったら
            phase = WarpPhase::Chase;
            enemy->bossChaseMarkModel->SetIsVisible(true);
            timer = 0.0f;
        }
        break;
    case WarpPhase::Chase:
    {
        // -------------------------
        // ボビン回避
        // -------------------------
        if (auto bobbin = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BobbinActor>())
        {
            DirectX::XMFLOAT3 bobbinPos = bobbin->GetPosition();

            // ボビンサイズ
            float bobbinRadius = 3.5f;

            float avoidDistance = bobbinRadius;

            DirectX::XMFLOAT3 toPos;
            toPos.x = pos.x - bobbinPos.x;
            toPos.z = pos.z - bobbinPos.z;
            toPos.y = 0.0f;

            float distSq =
                toPos.x * toPos.x +
                toPos.z * toPos.z;

            if (distSq < avoidDistance * avoidDistance)
            {
                float dist = sqrtf(distSq);

                // 真上防止
                if (dist < 0.001f)
                {
                    toPos = { 1.0f,0.0f,0.0f };
                    dist = 1.0f;
                }

                // 正規化
                toPos.x /= dist;
                toPos.z /= dist;

                // 押し出し
                pos.x = bobbinPos.x + toPos.x * avoidDistance;
                pos.z = bobbinPos.z + toPos.z * avoidDistance;
            }
        }
        if (auto player = enemy->GetPlayer())
        {
            DirectX::XMFLOAT3 playerPos = player->GetPosition();

            DirectX::XMFLOAT3 dir;
            dir.x = playerPos.x - pos.x;
            dir.z = playerPos.z - pos.z;
            dir.y = 0.0f;

            dir = MathHelper::Normalize(dir);

            float moveSpeed = 5.0f;

            pos.x += dir.x * moveSpeed * deltaTime;
            pos.z += dir.z * moveSpeed * deltaTime;

            // 出現範囲の半径
            float spawnAttackRange = enemy->GetAttackRange();

            float spawnMaxX = ScissorsGameState::stageMaxX - spawnAttackRange;
            float spawnMaxZ = ScissorsGameState::stageMaxZ - spawnAttackRange;

            float spawnMinX = ScissorsGameState::stageMinX + spawnAttackRange;
            float spawnMinZ = ScissorsGameState::stageMinZ + spawnAttackRange;

            // ステージ範囲制限
            pos.x = std::clamp(pos.x, spawnMinX, spawnMaxX);
            pos.z = std::clamp(pos.z, spawnMinZ, spawnMaxZ);

            enemy->SetPosition(pos);
        }
        if (timer > chaseTime)
        {
            phase = WarpPhase::ChaseEnd;
            timer = 0.0f;
        }
        // 追尾時に回転
        auto rot = enemy->bossChaseMarkModel->GetRelativeEulerRotation();
        rot.y += 15.0f * deltaTime;
        enemy->bossChaseMarkModel->SetRelativeEulerRotationDirect(rot);

        // 追尾マークの鼓動
        float baseScale = enemy->GetSpawnScale();
        float pulse = sinf(timer * 5.0f) * 0.2f + baseScale;

        enemy->bossChaseMarkModel->SetRelativeScaleDirect({
            pulse,
            pulse,
            pulse
            });
    }
    break;
    case WarpPhase::ChaseEnd:
    {
        float t = timer / chaseEndTime;
        t = std::clamp(t, 0.0f, 1.0f);

        float maxScale = enemy->GetSpawnScale();
        // 縮小
        float scale = std::lerp(maxScale, 0.0f, t);

        enemy->bossChaseMarkModel->SetRelativeScaleDirect({
            scale,
            scale,
            scale
            });

        // 回転を加速
        auto rot = enemy->bossChaseMarkModel->GetRelativeEulerRotation();
        rot.y += 30.0f * deltaTime;
        enemy->bossChaseMarkModel->SetRelativeEulerRotationDirect(rot);

        if (timer > chaseEndTime)
        {
            enemy->bossChaseMarkModel->SetIsVisible(false);

            // 出現予告へ
            phase = WarpPhase::Warning;
            timer = 0.0f;

            // 出現予告開始
            enemy->bossSpawnMarkModel->SetRelativeScaleDirect({ 0.0f,0.0f,0.0f });
        }
    }
    break;
    case WarpPhase::Warning:
    {
        enemy->bossSpawnMarkModel->SetIsVisible(true);
        // 場所が確定したら大きくしながら回転する
        float t = timer / warningTime;
        t = std::clamp(t, 0.0f, 1.0f);
        float maxScale = enemy->GetSpawnScale();
        float scale = std::lerp(0.0f, maxScale, t);
        enemy->bossSpawnMarkModel->SetRelativeScaleDirect({ scale,scale,scale });
        // 回転
        auto rot = enemy->bossSpawnMarkModel->GetRelativeEulerRotation();
        rot.y += 10.0f * deltaTime;
        enemy->bossSpawnMarkModel->SetRelativeEulerRotationDirect(rot);

        if (timer > warningTime)
        {
            phase = WarpPhase::Emerge;
            timer = 0.0f;
            enemy->StartEmerge();
            enemy->PlayAnimation("WarpEnd", false, true, 0.5f);
            enemy->SetAnimationRate(1.f);

        }
    }
    break;
    case WarpPhase::Emerge:

        if (enemy->IsFinishedEmerge())
        {
            // 出現ダメージ
            enemy->ApplyLandingDamage();
            // 当たり判定を有効にする
            enemy->collisionBoxComponent->EnableCollision();

            enemy->GetStateMachine()->ChangeState("Idle");
        }
        break;
    }

    DirectX::XMFLOAT3 markPos = pos;
    markPos.y = 0.0f;
    enemy->bossSpawnMarkModel->SetWorldLocationDirect(markPos);
    enemy->bossChaseMarkModel->SetWorldLocationDirect(markPos);
}

void RabbitBossAttackWarpState::Exit()
{
    // スポーンの見た目を無効にする
    enemy->bossSpawnMarkModel->SetIsVisible(false);
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

    // 全ての敵の玉止めする
    enemy->ApplyTiedAllEnemy();
}

void RabbitBossStunState::Execute(float deltaTime)
{
    stunTimer -= deltaTime;
    if (stunTimer < 0.0f)
    { // 待機を挟まず即ワープ
        enemy->GetStateMachine()->ChangeState("WarpPreview");
    }
}

void RabbitBossStunState::Exit()
{
    enemy->stunModel->SetIsVisible(false);

    // 全ての敵の玉止めを外す
}


// 死亡
void RabbitBossDeathState::Enter()
{
    enemy->collisionBoxComponent->DisableCollision();
    elapsedTime = 0.0f;
    // ボスが死亡したら呼ぶ処理  一フレームのみ
    enemy->StartDeathPerform();

    // 敵の出現を終了させる
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }
}

void RabbitBossDeathState::Execute(float deltaTime)
{
    // 死亡の演出を何か入れる
    enemy->UpdateDead(deltaTime);

    elapsedTime += deltaTime;

    //const float deadPerformTimeInterval = 5.0f;
    const float deadPerformTimeInterval = 0.0f;
    if (elapsedTime >= deadPerformTimeInterval)
    {
        enemy->EndDeathPerform();
    }
}

void RabbitBossDeathState::Exit()
{
}

// 勝利オブジェクト
void RabbitBossWinState::Enter()
{
    enemy->collisionBoxComponent->DisableCollision();
    elapsedTime = 0.0f;

    // 敵の出現を終了させる
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }

    enemy->PlayAnimation("Win", true, true, 0.5f);
}

void RabbitBossWinState::Execute(float deltaTime)
{
    // 勝利の演出を何か入れる
    enemy->UpdateWin(deltaTime);

    elapsedTime += deltaTime;

    //const float deadPerformTimeInterval = 5.0f;
    const float deadPerformTimeInterval = 1.0f;
    if (elapsedTime >= deadPerformTimeInterval)
    {
        enemy->EndDeathPerform();
    }
}

void RabbitBossWinState::Exit()
{
}

