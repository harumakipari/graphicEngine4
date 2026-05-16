#include "pch.h"
#include "RabbitBossState.h"

#include "BobbinActor.h"
#include "BossSpawner.h"
#include "RabbitBossEnemy.h"
#include "ScissorsGameState.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Utility/Time.h"


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
#if 1
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
    for (int i = 0; i < 6; i++)
    {
        attackBag.push_back(BossAttackType::Warp);
    }

    // Buff 1個
    for (int i = 0; i < 4; i++)
    {
        attackBag.push_back(BossAttackType::Buff);
    }
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
    // 沈みのSEを再生する
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/boss_warp_start.wav", 1.5f);
}

// ワープ
void RabbitBossAttackWarpState::Enter()
{
    // 地面の下に潜り始める
    enemy->isUnderGround = true;

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
            // 地面の下に潜り終えた
            enemy->isUnderGround = false;

            // 出現のSEを再生する
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/boss_warp_end.wav", 0.6f);
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
    // バフを掛ける前の音を再生する
    enemy->bossPreBuffAudioComponent->Play();

    enemy->SetAnimationRate(0.5f);
    enemy->PlayAnimation("PreBuff", false, true, 0.1f);

    // バフをかける敵をランダムに選ぶ
    enemy->EnlargeRandomEnemies(enemyBuffCount);
    // 経過時間をリセットする
    elapsedTime = 0.0f;
}

void RabbitBossAttackBuffPreviewState::Execute(float deltaTime)
{
    elapsedTime += deltaTime;
    if (elapsedTime >= 2.0f)
    {
        enemy->GetStateMachine()->ChangeState("Buff");
    }
}

void RabbitBossAttackBuffPreviewState::Exit()
{
    enemy->bossPreBuffAudioComponent->Stop();
}

// バフ
void RabbitBossAttackBuffState::Enter()
{
    // 経過時間をリセットする
    elapsedTime = 0.0f;

    enemy->SetAnimationRate(1.0f);
    enemy->PlayAnimation("Buff", false, true, 0.1f);

    // パワーアップを知らせる音を鳴らす
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/enemy_power_up.wav", 0.8f);
}

void RabbitBossAttackBuffState::Execute(float deltaTime)
{
    elapsedTime += deltaTime;
    if (elapsedTime >= 1.0f)
    {
        enemy->PlayAnimation("Idle", true, true, 0.5f);
        enemy->GetStateMachine()->ChangeState("Idle");
    }
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

    // スタンのアニメーション
    enemy->SetAnimationRate(1.5f);
    enemy->PlayAnimation("Stan", false, true, 0.1f);

    // スタン時の音を再生する
    enemy->bossStunAudioComponent->Play();

    // 敵の出現を終了させる
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }

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

    // アニメーションの倍率を戻す
    enemy->SetAnimationRate(1.0f);

    // スタン時の音を止める
    enemy->bossStunAudioComponent->Stop();

    // 再スタン防止開始
    enemy->stunCooldownTimer = enemy->stunCooldownDuration;

    // 敵の出現を開始する
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Activate();
    }

}


// 死亡
void RabbitBossDeathState::Enter()
{
    enemy->collisionBoxComponent->DisableCollision();

    elapsedTime = 0.0f;

    // ボスが死亡したら呼ぶ処理  一フレームのみ 敵全員死亡の演出を開始する
    enemy->StartDeathPerform();

    // 敵の出現を終了させる
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }

    {// ここで経過時間を停止する
        enemy->EndDeathPerform(true);// 引数にFinishUIを出さないかどうか
    }
    // スロー開始
    Time::timeScale = 0.5f;

    enemy->PlayAnimation("KnockBack", false, true, 0.1f);

    phase = DeathPhase::StartSlow;

    // 周りにあるモデルを非表示にする
    enemy->HideAroundModel();

    // ボビンの見た目を非表示にする
    if (auto bobbin = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BobbinActor>())
    {
        bobbin->HideBobbinVisual();
    }

}

void RabbitBossDeathState::Execute(float deltaTime)
{
    elapsedTime += deltaTime;
    // 死亡の演出を何か入れる
    enemy->UpdateDead(deltaTime);


    switch (phase)
    {
    case DeathPhase::StartSlow:
        if (elapsedTime > 0.8f)
        {
            phase = DeathPhase::KnockBack;
            // スロー開始
            Time::timeScale = 1.0f;

        }
        break;

    case DeathPhase::KnockBack:
        if (!enemy->GetAnimationController()->IsPlayAnimation())
        {
            phase = DeathPhase::CameraMove;
        }
        break;

    case DeathPhase::CameraMove:
    {
        cameraLerpT += Time::UnscaledDeltaTime() * 0.5f;

        //enemy->GetSceneCamera()->LerpToTarget(
        //    enemy->GetPosition(),
        //    cameraLerpT
        //);

        if (cameraLerpT >= 0.0f)
        {
            phase = DeathPhase::Stun;
            //enemy->PlayAnimation("Stun", false, true, 0.1f);
            // スタンのアニメーション
            enemy->SetAnimationRate(1.5f);
            enemy->PlayAnimation("Stan", false, true, 0.1f);

        }
        break;
    }

    case DeathPhase::Stun:
        if (!enemy->GetAnimationController()->IsPlayAnimation())
        {
            phase = DeathPhase::Tear;
            //enemy->SpawnTearEffect();
        }
        break;

    case DeathPhase::Tear:
        if (elapsedTime > 3.0f)
        {
            phase = DeathPhase::Finish;
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "ResultScene"), std::make_pair("fromScene","GameScene") });
        }
        break;

    case DeathPhase::Finish:
        break;
    }
}


void RabbitBossDeathState::Exit()
{
}

// 勝利オブジェクト
void RabbitBossWinState::Enter()
{
    enemy->collisionBoxComponent->DisableCollision();

    phase = BossWinPhase::WaitCircleShrink;

    // 地中にいるかを判断
    requireEmerge = enemy->isUnderGround;

    // 半径を設定
    startRadius = 1.2f;

    // 収縮までにかかる時間を設定
    duration = 1.5f;
    elapsedTime = 0.0f;

    //　ボスの位置でゲームオーバーの半径を決定
    float z = enemy->GetPosition().z;
    targetRadius = (z > 12.0f) ? 0.25f : 0.3f;

    // 敵の出現を終了させる
    if (auto bossSpawner = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>())
    {
        bossSpawner->Deactivate();
    }

    // ここで時間を停止する
    enemy->EndDeathPerform(true);// 引数にプレイヤーが死亡したかどうか

    // 周りにあるモデルを非表示にする
    enemy->HideAroundModel();

    // ボビンの見た目を非表示にする
    if (auto bobbin = enemy->GetOwnerScene()->GetActorManager()->GetActorOfType<BobbinActor>())
    {
        bobbin->HideBobbinVisual();
    }
}

void RabbitBossWinState::Execute(float deltaTime)
{

    switch (phase)
    {
    case BossWinPhase::WaitCircleShrink:
    {
        elapsedTime += deltaTime;
        float t = elapsedTime / duration;

        t = std::clamp(t, 0.0f, 1.0f);

        float radius = std::lerp(startRadius, targetRadius, t);
        //Logger::Log(U8("ゲームオーバー半径") + std::to_string(radius));


        enemy->SetDeathRadius(radius);

        // 勝利の演出を何か入れる
        enemy->UpdateWin(deltaTime);

        if (t >= 1.0f)
        {
            if (requireEmerge)
            {
                phase = BossWinPhase::Emerge;

                enemy->StartEmerge();

                enemy->PlayAnimation(
                    "WarpEnd",
                    false,
                    true,
                    0.5f
                );

                enemy->SetAnimationRate(1.f);

                CoreAudio::PlayOneShot(
                    L"./Data/Sound/SE1/boss_warp_end.wav",
                    0.6f
                );
            }
            else
            {
                phase = BossWinPhase::WinAnimation;

                elapsedTime = 0.0f;

                // 勝利のアニメーション
                enemy->PlayAnimation(
                    "Win",
                    true,
                    true,
                    0.5f
                );
                // ボスの笑い後のSEを再生する
                CoreAudio::PlayOneShot(
                    L"./Data/Sound/SE1/boss_laugh.wav",
                    0.8f
                );
            }
        }

        break;
    }
    case BossWinPhase::Emerge:
    {
        if (enemy->IsFinishedEmerge())
        {
            phase = BossWinPhase::WinAnimation;

            elapsedTime = 0.0f;

            enemy->PlayAnimation(
                "Win",
                true,
                true,
                0.5f
            );
            // ボスの笑い後のSEを再生する
            CoreAudio::PlayOneShot(
                L"./Data/Sound/SE1/boss_laugh.wav",
                0.8f
            );
        }
    }
    break;
    case BossWinPhase::WinAnimation:
    {
        enemy->UpdateWin(deltaTime);

        elapsedTime += deltaTime;
        const float winTime = 1.5f; // 何秒待つか

        if (elapsedTime >= winTime)
        {
            phase = BossWinPhase::CloseCircle;

            elapsedTime = 0.0f;
            // シーン遷移のための半径を設定する
            startRadius = enemy->GetDeathRadius();
            targetRadius = -0.1f;
            duration = 0.4f;
        }

        break;
    }
    case BossWinPhase::CloseCircle:
    {

        elapsedTime += deltaTime;

        float t = elapsedTime / duration;

        t = std::clamp(t, 0.0f, 1.0f);

        float radius =
            std::lerp(startRadius, targetRadius, t);
        //Logger::Log(U8("ゲームオーバー半径") + std::to_string(radius));

        enemy->SetDeathRadius(radius);

        enemy->UpdateWin(deltaTime);

        if (t >= 1.0f)
        {
            //Logger::Log(U8("シーン遷移"));
            SceneTransitionManager::Instance().RequestTransition(
                "LoadingScene",
                {
                    std::make_pair("preload", "ResultScene"),
                    std::make_pair("fade","0"),
                    std::make_pair("fromScene","GameScene")
                },
                TransitionStyle::Fade
            );
            phase = BossWinPhase::End;
        }
        break;
    }
    case BossWinPhase::End:
        break;
    }

}

void RabbitBossWinState::Exit()
{
}

