#include "pch.h"
#include "EnemyBase.h"

#include "ButtonCoinActor.h"
#include "ScissorsGameElasticComponent.h"
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
        if (attack)
        {
            attack->Update(this, deltaTime);
        }
        UpdateScissors(deltaTime);
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

void EnemyBase::DrawImGuiDetails()
{
    if (ImGui::Button(U8("敵のサイズを大きくする")))
    {
        ChangeSize(YarnSize::Big);
    }
    if (ImGui::Button(U8("敵に力を加える")))
    {
        //elasticMeshComponent->AddImpulse({1, 1, 1});
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
#if 0
    static float totalTime = 0.0f;
    totalTime += deltaTime;

    // ===== 揺れ設定 =====
    float shakeAmp = (size == YarnSize::Big) ? 0.15f : 0.07f;
    float shakeSpeed = 20.0f;

    float noiseX = sinf(totalTime * shakeSpeed + 1.0f) * shakeAmp;
    float noiseZ = sinf(totalTime * shakeSpeed + 2.3f) * shakeAmp;

    // 元位置をベースにする
    XMFLOAT3 basePos = GetPosition();

    // ※ここ重要：元の位置を保持しておく変数を使うのが理想
    // もし無いなら startPosition を使う
    basePos = startPosition;

    XMFLOAT3 shakenPos = basePos;
    shakenPos.x += noiseX;
    shakenPos.z += noiseZ;

    SetPosition(shakenPos);

#endif // 0
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

// ハサミの角度を変更する処理
void EnemyBase::UpdateScissors(float deltaTime)
{
    if (!scissorsFirstMeshComponent || !scissorsSecondMeshComponent) return;
    scissorsAnimTime += deltaTime;

    float angle = 0.0f;
    if (!isRescuing)
    {
        //  徘徊中
        float wave = abs(sin(scissorsAnimTime * 3.0f)) * 10.0f;
        float base = 5.0f;
        angle = base + wave;
    }
    else if (isCutting)
    {
        scissorsCutTimer += deltaTime;

        float duration = 0.2f; // 切る速さ（短いほどキレがいい）
        float t = scissorsCutTimer / duration;

        if (t >= 1.0f)
        {
            t = 1.0f;
        }

        // 30 → 0 に補間
        angle = 30.0f * (1.0f - t);
    }
    else
    {
        angle = 0.0f;
    }

    scissorsFirstMeshComponent->SetRelativeEulerRotationDirect({ 0, angle, 0 });
    scissorsSecondMeshComponent->SetRelativeEulerRotationDirect({ 0, -angle, 0 });

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

// 攻撃セット関数
void EnemyBase::SetAttack(std::unique_ptr<EnemyAttack> newAttack)
{
    attack = std::move(newAttack);
}


// プレイヤーを取得する
ScissorsPlayer1* EnemyBase::GetPlayer()
{
    auto actorManager = GetOwnerScene()->GetActorManager();
    if (auto player = actorManager->GetActorOfType<ScissorsPlayer1>())
    {
        return player.get();
    }
    return nullptr;
}

// ハサミを生成する
void EnemyBase::CreateScissorsVisual()
{
    if (scissorsFirstMeshComponent.get() && scissorsSecondMeshComponent.get()) return; // 既にあるなら何もしない

    scissorsFirstMeshComponent = AddComponent<SkeletalMeshComponent>("ScissorsFirstModel", parentName);
    scissorsFirstMeshComponent->SetRelativeLocationDirect({ 0, 1.0f, 0 });
    scissorsFirstMeshComponent->SetRelativeScaleDirect({ 1.0f,1.0f,1.0f });
    scissorsFirstMeshComponent->SetModel("./Data/TeamModels/Item/ScissorsFirstModel.glb");

    scissorsSecondMeshComponent = AddComponent<SkeletalMeshComponent>("ScissorsSecondModel", parentName);
    scissorsSecondMeshComponent->SetRelativeLocationDirect({ 0, 1.0f, 0 });
    scissorsSecondMeshComponent->SetRelativeScaleDirect({ 1.0f,1.0f,1.0f });
    scissorsSecondMeshComponent->SetModel("./Data/TeamModels/Item/ScissorsSecondModel.glb");
}


void EnemyBase::SetUpVisual()
{
    if (enemyType == YarnEnemyType::LongRangeAttack)
    {
        // ハリネズミモデル
        skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/NeedleEnemy.glb", false, true);
        skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

        // 当たり判定
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
        height = size.y;
        mass = 0.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetStatic(true);
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
        tiedLeft->SetModel("./Data/TeamModels/Item/tiedModelLeftBig.glb", false, true);
        tiedLeft->SetRelativeScaleDirect({ 1.2f,1.2f,1.2f });
        tiedLeft->SetIsVisible(false);
        tiedLeft->SetIsCastShadow(false);
        tiedMeshes.push_back(tiedLeft);

        auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
        tiedRight->SetModel("./Data/TeamModels/Item/tiedModelRightBig.glb", false, true);
        tiedRight->SetRelativeScaleDirect({ 1.2f,1.2f,1.2f });
        tiedRight->SetIsVisible(false);
        tiedRight->SetIsCastShadow(false);
        tiedMeshes.push_back(tiedRight);


        // 位置を更新　当たり判定が{0,0,0}にくるのを防ぐため
        UpdateAllComponentTransforms();
        return;
    }

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
        mass = 0.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(mass);
        sphereCollisionComponent->SetStatic(true);
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
        tiedLeft->SetIsCastShadow(false);
        tiedMeshes.push_back(tiedLeft);

        auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
        tiedRight->SetModel("./Data/TeamModels/Item/tiedModelRight.glb", false, true);
        tiedRight->SetIsVisible(false);
        tiedRight->SetIsCastShadow(false);
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
        skeletalMeshComponent->SetIsVisible(false);

        elasticMeshComponent = AddComponent<ScissorsGameElasticMeshComponent>(parentName);
        elasticMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnBigEnemy.glb", false, true);
        elasticMeshComponent->Initialize();

        // 当たり判定
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
        height = size.y;
        mass = 0.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetStatic(true);
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
        tiedLeft->SetIsCastShadow(false);
        tiedMeshes.push_back(tiedLeft);

        auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
        tiedRight->SetModel("./Data/TeamModels/Item/tiedModelRightBig.glb", false, true);
        tiedRight->SetIsVisible(false);
        tiedRight->SetIsCastShadow(false);
        tiedMeshes.push_back(tiedRight);

    }
    break;
    }

    // 位置を更新　当たり判定が{0,0,0}にくるのを防ぐため
    UpdateAllComponentTransforms();

}


// 敵のサイズを変更する
void EnemyBase::ChangeSize(YarnSize newSize)
{
    if (size == newSize)
    {// 現在のサイズと同じだったら
        Logger::Log(U8(""));
        return;
    }
    size = newSize;

    // 玉止め解除
    ReleasedTied();

#if 1
    // 既存コンポーネントを削除
    RequestDestroyComponent("sphereComponent");
    RequestDestroyComponent("tiedLeftMeshComponent");
    RequestDestroyComponent("tiedRightMeshComponent");
    tiedMeshes.clear();
#endif // 0


    // 見た目・当たり判定を再生成
    SetUpVisual();
    UpdateAllComponentTransforms();
}