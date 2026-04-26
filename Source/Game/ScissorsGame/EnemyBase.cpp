#include "pch.h"
#include "EnemyBase.h"

#include "ButtonCoinActor.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Game/Scenes/GameScene.h"
#include "Physics/CollisionFunction.h"

void EnemyBase::Initialize(const Transform& transform)
{
    parentName = "EnemyBase";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };

    state = YarnState::Active;
    SetBehavior(std::make_unique<StaticBehavior>());

}

void EnemyBase::Update(float deltaTime)
{
    // 玉止めの描画更新処理
    UpdateTiedVisual();


    switch (state)
    {
    case YarnState::Active:
        if (behavior)
        {
            behavior->Update(this, deltaTime);
        }
        break;
    case YarnState::Tied:
        UpdateTied(deltaTime);
        break;
    case YarnState::Dead:
        UpdateDead(deltaTime);
        break;
    }

    // 反射の当たり判定をデバッグ表示
    //DebugRender::DrawSphere(redirectLeftCollisionComponent->GetComponentLocation(), dashAttackRange, { 1,1,0,1 }, 0, true);

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
            // 振動させる
            InputSystem::SetVibration(1.0f, 0.15f);
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
    if (size == YarnSize::Big)
    {// 大きい敵だったら
        tieTimer += deltaTime;

        if (tieTimer > selfRescueTimeInterval)
        {// 自力で脱出する
            ReleasedTied();
        }
    }
}

// 玉止め表示更新処理
void EnemyBase::UpdateTiedVisual()
{
    int showCount = 0;

    if (size == YarnSize::Small)
    {
        if (tieCount == 1)
        {
            showCount = 2; // 小は1回で2個
        }
    }
    else if (size == YarnSize::Big)
    {
        if (tieCount == 1) showCount = 1;
        else if (tieCount >= 2) showCount = 2;
    }

    for (int i = 0; i < tiedMeshes.size(); i++)
    {
        tiedMeshes[i]->SetIsVisible(i < showCount);
        tiedMeshes[i]->SetRelativeEulerRotationDirect({ 0.0f,180.0f,0.0f });
    }
}

// 玉止めをほどく
void EnemyBase::ReleasedTied()
{
    tieCount = 0;
    state = YarnState::Active;

    for (auto& tied : tiedMeshes)
    {
        tied->SetIsVisible(false);
    }
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

// サイズのセット関数
void EnemyBase::SetEnemySize(const YarnSize size)
{
    this->size = size;

    switch (size)
    {
    case Small:
    {
        // スコアの設定
        scoreData = { 100,0 };

        skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnEnemy.glb", false, true);
        skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

        // 当たり判定
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
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

        // 玉止めモデル
        auto tiedLeft = AddComponent<SkeletalMeshComponent>("tiedLeftMeshComponent", parentName);
        tiedLeft->SetModel("./Data/TeamModels/Item/tiedModelLeft.glb", false, true);
        tiedLeft->SetIsVisible(false);
        tiedMeshes.push_back(tiedLeft);

        auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
        tiedRight->SetModel("./Data/TeamModels/Item/tiedModelRight.glb", false, true);
        tiedRight->SetIsVisible(false);
        tiedMeshes.push_back(tiedRight);

    }
    break;
    case Big:
    {
        // スコアの設定
        scoreData = { 200,0 };

        skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnBigEnemy.glb", false, true);
        skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

        // 当たり判定
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
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

#if 0
        // 反射用の当たり判定
        redirectCollisionComponent = this->AddComponent<class SphereComponent>("redirectLeftCollisionComponent", parentName);
        //redirectLeftCollisionComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        redirectCollisionComponent->SetRadius(radius);
        redirectCollisionComponent->SetMass(mass);
        redirectCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        redirectCollisionComponent->SetLayer(CollisionLayer::EnemyRedirect);
        redirectCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        redirectCollisionComponent->Initialize();
#endif // 0

        // 玉止めモデル
        auto tiedLeft = AddComponent<SkeletalMeshComponent>("tiedLeftMeshComponent", parentName);
        tiedLeft->SetModel("./Data/TeamModels/Item/tiedModelLeftBig.glb", false, true);
        tiedLeft->SetIsVisible(false);
        tiedMeshes.push_back(tiedLeft);

        auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
        tiedRight->SetModel("./Data/TeamModels/Item/tiedModelRightBig.glb", false, true);
        tiedRight->SetIsVisible(false);
        tiedMeshes.push_back(tiedRight);

    }
    break;
    }

    // 位置を更新　当たり判定が{0,0,0}にくるのを防ぐため
    UpdateAllComponentTransforms();

}

// プレイヤーを取得する
ScissorsPlayer1* EnemyBase::GetPlayer()
{
    auto actorManager = GetOwnerScene()->GetActorManager();
    if (auto player=actorManager->GetActorOfType<ScissorsPlayer1>())
    {
        return player.get();
    }
    return nullptr;
}

// ハサミを生成する
void EnemyBase::EnableScissorsVisual()
{
    if (scissorsMeshComponent.get()) return; // 既にあるなら何もしない

    scissorsMeshComponent = AddComponent<SkeletalMeshComponent>("ScissorsMesh",parentName);
    scissorsMeshComponent->SetRelativeLocationDirect({ 0, 1.0f, 0 });
    scissorsMeshComponent->SetRelativeScaleDirect({ 0.5f,0.5f,0.5f });
    scissorsMeshComponent->SetModel("./Data/TeamModels/Item/ScissorsModel.glb");
}