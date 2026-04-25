#include "pch.h"
#include "ScissorsGameEnemyBaseActor.h"

#include "ButtonCoinActor.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Game/Scenes/GameScene.h"
#include "Physics/CollisionFunction.h"

void ScissorsGameEnemyBase::Initialize(const Transform& transform)
{
}

void ScissorsGameEnemyBase::Update(float deltaTime)
{
    if (state == YarnState::Dead)
        return;

    if (state == YarnState::Tied)
    {
        tieTimer += deltaTime;

#if 0
        // 一定時間で自力解除
        if (tieTimer > 3.0f)
        {
            ReleaseTie();
        }

#endif // 0
        return; // 動かない
    }


    if (isDead)
    {// 死亡したら
        // 上へ吹っ飛ぶ処理
        if (isKnockbackActive)
        {
            if (knockback.elapsedTime < 0.05f)
            {
                // 少しだけ強制的に前に押す
                XMFLOAT3 pos = GetPosition();
                pos.x += (knockback.targetPos.x - knockback.startPos.x) * 0.1f;
                pos.z += (knockback.targetPos.z - knockback.startPos.z) * 0.1f;
                SetPosition(pos);
            }

            knockback.elapsedTime += deltaTime;

            float t = knockback.elapsedTime / knockback.duration;
            t = std::clamp(t, 0.0f, 1.0f);
            float easedT = 1.0f - powf(1.0f - t, 5.0f);

            // 線形補間（XZ）
            XMFLOAT3 pos;
            pos.x = std::lerp(knockback.startPos.x, knockback.targetPos.x, easedT);
            pos.z = std::lerp(knockback.startPos.z, knockback.targetPos.z, easedT);

            // 放物線（Y）
            float yT = powf(t, 0.7f); // 最初から上がる
            float height = 4.0f * knockback.height * yT * (1.0f - yT);
            pos.y = knockback.startPos.y + height;

            SetPosition(pos);

            // 高さでコイン出す
            if (pos.y > knockback.startPos.y + knockback.height * 0.8f && !createCoin)
            {
                SpawnCoin(pos);
                createCoin = true;
            }
            // 終了
            if (t >= 1.0f)
            {
                MarkPendingKill(); // 死亡処理はエフェクトが終わってからにする予定
                isKnockbackActive = false;
            }
        }
        // 白くフラッシュする処理
        {
            hitFlashTimer += deltaTime;
#if 0
            float t = (1.0f - hitFlashTimer / hitFlashDuration);
            t = std::clamp(t, 0.0f, 1.0f);
#else
            float t = hitFlashTimer / hitFlashDuration;
            t = std::clamp(t, 0.0f, 1.0f);

            // 急激に減衰
            t = powf(1.0f - t, 7.0f);
#endif // 0

            skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = t;

            if (t >= 1.0f)
            {
                skeletalMeshComponent->SetIsVisible(false);
            }
        }
    }



}

void ScissorsGameEnemyBase::MoveLinear(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ステージ端で反転
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;

    if (pos.x < stageMinX || pos.x > stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
    }

    if (pos.z < stageMinZ || pos.z > stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);
    }

    SetPosition(pos);

    rotationComponent->SetDirection(moveDirection);

}

bool ScissorsGameEnemyBase::OnHitByDash(ScissorsPlayer1* player, int dashDamage)
{
    if (state == YarnState::Dead) return false;

    tieCount++;

    // 玉止めに必要な回数を取得する
    int needTie = GetNeedTiedCount();

    if (tieCount >= needTie)
    {
        if (state == YarnState::Tied)
        {
            // 2回目 → 死亡
            state = YarnState::Dead;
            CallDeath(true);
            return true;
        }
        else
        {
            // 1回目 → 球止め
            state = YarnState::Tied;
            tieTimer = 0.0f;
        }
    }

    InputSystem::SetVibration(1.0f, 0.15f);
    return false;


    int prevHp = hp;
    // ダッシュで当たったときの処理
    TakeDamage(dashDamage, true);
    // コントローラーを振動させる
    InputSystem::SetVibration(0.8f, 0.15f);

    // 倒したかどうかを返す
    return (hp <= 0 && prevHp > 0);
}

// プレイヤーのハサミ攻撃が当たったときの処理
bool ScissorsGameEnemyBase::OnHitByAttack(ScissorsPlayer1* player, int dashDamage)
{
    int prevHp = hp;
    // ダッシュで当たったときの処理
    TakeDamage(dashDamage, false);
    // コントローラーを振動させる
    InputSystem::SetVibration(0.8f, 0.15f);

    // 倒したかどうかを返す
    return (hp <= 0 && prevHp > 0);
}

// ダメージを与える　死亡したかどうかを取得する関数
bool ScissorsGameEnemyBase::TakeDamage(int damage, bool hitByDash)
{
    if (hp <= 0) return true; // すでに倒れている場合は無視

    hp -= damage;
    Logger::Log(U8("敵にダメージ：") + std::to_string(damage));
    if (hp <= 0)
    {
        CallDeath(hitByDash);

#if 0 // 吹っ飛ばす前にActorを消す
        MarkPendingKill();
#endif // 0 // 吹っ飛ばす前にActorを消す
        if (starEffectComponent)
        {
            //starEffectComponent->Play();
        }
        return true;
    }
    return false;

}

// ヒットエフェクトを生成する
void ScissorsGameEnemyBase::SpawnHitEffect(bool hitByDash)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    // ワールド→スクリーン変換（必要なら）
    XMFLOAT3 pos = GetPosition();
    XMFLOAT2 screenPos = WorldToUI(pos);

    XMFLOAT4 endColor = { 1,1,0.3f,0.5f };

    if (!hitByDash)
    {// 突進によって死亡していない場合  
        endColor = { 0.96f,0.51f,0.125f,0.5f }; //オレンジ色
        endColor = { 0.16f,0.81f,0.9f,0.5f };
    }

    // リング（少し遅らせると良い）
    auto ring = std::make_shared<UIRingEffect>("./Data/Textures/ScissorsUI/ring.png", endColor);
    ring->SetWorldPosition(screenPos);
    ring->SetSize({ 100,100 });
    uiManager->Add(ring);

    // 星
    for (int i = 0; i < 8; i++)
    {
        auto star = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png", screenPos);
        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }
}

void ScissorsGameEnemyBase::ApplyKnockBack(DirectX::XMFLOAT3 dir, float horizontalPower, float verticalPower)
{
    velocity.x = dir.x * horizontalPower;
    velocity.z = dir.z * horizontalPower;
    velocity.y = verticalPower;
}


// 死亡した時に呼ぶ関数
void ScissorsGameEnemyBase::CallDeath(bool hitByDash)
{
    isDead = true;

    if (onDeath)
    {// WaveManagerにenemyCountを減らすように通知する
        onDeath();
    }

    // エフェクトを発生させる
    SpawnHitEffect(hitByDash);

    // 当たり判定を消す
    if (sphereCollisionComponent)
    {
        sphereCollisionComponent->DisableCollision();
    }

    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        Logger::Error(U8("CallDeath関数内でプレイヤーがnullです"));
        return;
    }

    XMFLOAT3 playerPos = player->GetPosition();
    XMFLOAT3 start = GetPosition();

    // 吹っ飛ぶ方向
    XMFLOAT3 dir = MathHelper::Normalize(MathHelper::Subtract(start, playerPos));

    // 調整のために
    auto scene = static_cast<GameScene*>(GetOwnerScene());
    auto& tuning = scene->enemyTuning;

    float distance = hitByDash ? tuning.knockbackDistanceDash : tuning.knockbackDistanceNormal;
    float height = hitByDash ? tuning.knockbackHeightDash : tuning.knockbackHeightNormal;
    float duration = hitByDash ? tuning.knockbackDurationDash : tuning.knockbackDurationNormal;
    hitFlashDuration = tuning.flashDuration;
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = tuning.emissivePower;


    // 目標位置
    XMFLOAT3 target =
    {
        start.x + dir.x * distance,
        start.y,
        start.z + dir.z * distance
    };

    knockback = { start,target,height,duration,0.0f };
    isKnockbackActive = true;

    // 敵が白くなって薄くなってからまた元の色に戻る
    hitFlashTimer = 0.0f;


    // ある程度吹っ飛んだら敵が見えなくする

    createCoin = false;

}

// コインを生成する
void ScissorsGameEnemyBase::SpawnCoin(DirectX::XMFLOAT3 pos)
{
    // コインを生成する
    Transform coinTr(pos, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto coin = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ButtonCoinActor>("coin", coinTr);
    coin->StartPerform();
    createCoin = true;

}