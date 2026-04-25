#include "pch.h"
#include "EnemyBase.h"

#include "ButtonCoinActor.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Game/Scenes/GameScene.h"
#include "Physics/CollisionFunction.h"

void EnemyBase::Initialize(const Transform& transform)
{
    std::string parentName = "EnemyBase";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnEnemy.glb", false, true);
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

    // 当たり判定
    {
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(mass);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Enemy);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
    }

    // 玉止めモデル
    tiedMeshComponent = AddComponent<SkeletalMeshComponent>("tiedMeshComponent", parentName);
    tiedMeshComponent->SetModel("./Data/TeamModels/Effect/TiedModel.glb", false, true);
    tiedMeshComponent->SetIsVisible(false);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };

    state = YarnState::Active;
    SetBehavior(std::make_unique<IdleEnemyBehavior>());
}

void EnemyBase::Update(float deltaTime)
{
    // Dead
    if (state == YarnState::Dead)
    {
        UpdateDead(deltaTime);
        return;
    }

    // Tied
    if (state == YarnState::Tied)
    {
        UpdateTied(deltaTime);
        return;
    }

    // Active
    if (behavior)
    {
        behavior->Update(this, deltaTime);
    }
}

bool EnemyBase::OnHitByDash()
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

    // 振動させる
    InputSystem::SetVibration(1.0f, 0.15f);
    return false;
}

// ヒットエフェクトを生成する
void EnemyBase::SpawnHitEffect(bool hitByDash)
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

    // リング
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

// 玉止めされている時
void EnemyBase::UpdateTied(float deltaTime)
{
    tiedMeshComponent->SetIsVisible(true);
}

// 玉止めをほどく
void EnemyBase::ReleasedTiled()
{

}

// 死亡中の更新処理
void EnemyBase::UpdateDead(float deltaTime)
{
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



// 死亡した時に呼ぶ関数
void EnemyBase::CallDeath(bool hitByDash)
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

    // コインフラグをオフにしておく
    createCoin = false;
}

// コインを生成する
void EnemyBase::SpawnCoin(DirectX::XMFLOAT3 pos)
{
    // コインを生成する
    Transform coinTr(pos, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto coin = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ButtonCoinActor>("coin", coinTr);
    coin->StartPerform();
}


// 移動
void EnemyBase::Move(const DirectX::XMFLOAT3& dir, float deltaTime)
{
    auto pos = GetPosition();
    pos.x += dir.x * speed * deltaTime;
    pos.z += dir.z * speed * deltaTime;
    SetPosition(pos);
}

// 向き
void EnemyBase::Face(const DirectX::XMFLOAT3& dir)
{
    if (rotationComponent)
        rotationComponent->SetDirection(dir);
}

// セット関数
void EnemyBase::SetBehavior(std::unique_ptr<EnemyBehavior> newBehavior)
{
    if (behavior) behavior->Exit(this);

    behavior = std::move(newBehavior);

    if (behavior) behavior->Enter(this);
}
