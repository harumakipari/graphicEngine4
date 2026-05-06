#include "pch.h"
#include "EnemyBase.h"

#include "ButtonCoinActor.h"
#include "ScissorsGameElasticComponent.h"
#include "ScissorsPlayer1.h"
#include "ScorePopupActor.h"
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

    // サイズ変更演出
    if (isSizeChanging)
    {
        UpdateSizeChanging(deltaTime);
    }

    switch (state)
    {
    case YarnState::Active:
    case YarnState::Tying:
        if (behavior)
        {
            behavior->Update(this, deltaTime);
        }
        if (attack)
        {
            attack->Update(this, deltaTime);
        }
        // ハサミを持つ敵でハサミの更新処理
        UpdateScissors(deltaTime);

        // ベース位置を設定する
        basePosition = GetPosition();

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

bool EnemyBase::OnHitByDash(bool isReflected)
{
    if (state == YarnState::Dead) return false;

    tieCount++;

    // 玉止めに必要な回数を取得する
    int needTie = GetNeedTiedCount();

    if (tieCount >= needTie)
    {
        if (state == YarnState::Tied)
        {
            if (isReflected)
            {// 反射によって死亡したら
                pendingDeath = true;
            }
            else
            {
                // 振動させる
                InputSystem::SetVibration(0.8f, 0.15f);
                CallDeath(false);
            }
            // 2回目 → 死亡
            state = YarnState::Dead; // startKnockbackがtrueにならないと吹っ飛ばない

            return true;
        }
        else
        {
            // 1回目 → 球止め
            state = YarnState::Tied;
            tieTimer = 0.0f;
            shakeTimer = 0.0f; // 振動のタイマーをリセットする
            selfRescueTimeInterval = (yarnSize == YarnSize::Big) ? selfBigRescueTimeInterval : selfSmallRescueTimeInterval; // 自力脱出にかかる時間を設定
        }
    }

    return false;
}

// 強制的に玉止めする
void EnemyBase::ForceTied()
{
    if (state == YarnState::Dead)
        return;

#if 0
    //　既に玉止め状態の場合
    if (state == YarnState::Tying)
        return; // 一旦何もしない
#endif // 0

    state = YarnState::Tied;
    tieTimer = 0.0f;
    selfRescueTimeInterval = (yarnSize == YarnSize::Big) ? selfBigRescueTimeInterval : selfSmallRescueTimeInterval; // 自力脱出にかかる時間を設定
    shakeTimer = 0.0f; // 振動のタイマーをリセットする

    tieCount = GetNeedTiedCount();
}

// スコアポップアップを生成する関数
void EnemyBase::SpawnScorePopup(const DirectX::XMFLOAT3& pos, int score)
{
    Transform scoreTr{ pos,XMFLOAT3{0,0,0},XMFLOAT3{1,1,1} };
    auto popup = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ScorePopupActor>("ScorePopup", scoreTr);
    popup->SetScore(score);
}

// ヒットエフェクトを生成する
void EnemyBase::SpawnHitEffect(bool hitByReflected)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    // ワールド→スクリーン変換（必要なら）
    XMFLOAT3 pos = GetPosition();
    XMFLOAT2 screenPos = WorldToUI(pos);

    XMFLOAT4 endColor = { 1,1,0.3f,0.5f };
    XMFLOAT2 endSize = { 400.0f, 400.0f }; // リングサイズ

    if (hitByReflected)
    {// 反射によって死亡した場合
        endColor = { 0.16f,0.81f,0.9f,0.5f };   // 水色
        endColor = { 0.93f,0.1f,0.24f,0.5f }; //赤色
        endSize = { 600.0f,600.f };
    }

    // リング
    auto ring = std::make_shared<UIRingEffect>("./Data/Textures/ScissorsUI/ring.png", endColor);
    ring->SetWorldPosition(screenPos);
    ring->SetSize({ 100,100 });
    ring->SetEndSize(endSize);

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
#if 0// 玉止め抜ける時の揺れ
    shakeTimer += deltaTime;


    // ===== 揺れ設定 =====
    float shakeAmp = (yarnSize == YarnSize::Big) ? 0.15f : 0.07f;
    float shakeSpeed = 20.0f;

    float noiseX = sinf(shakeTimer * shakeSpeed + 1.0f) * shakeAmp;
    float noiseZ = sinf(shakeTimer * shakeSpeed + 2.3f) * shakeAmp;

    XMFLOAT3 shakenPos = basePosition;
    shakenPos.x += noiseX;
    shakenPos.z += noiseZ;

    SetPosition(shakenPos);

#endif // 0
    tieTimer += deltaTime;

    float timeLeft = selfRescueTimeInterval - tieTimer;

    //  5秒前から揺れ開始
    if (timeLeft <= 5.0f)
    {
        float shakeRatio = 1.0f - (timeLeft / 5.0f); // 0 → 1 に増える
        shakeTimer += deltaTime;

        float baseAmp = (yarnSize == YarnSize::Big) ? 0.15f : 0.07f;
        float shakeAmp = baseAmp * shakeRatio; // 徐々に強く

        float shakeSpeed = 20.0f + 30.0f * shakeRatio; // 徐々に速く

        float noiseX = sinf(shakeTimer * shakeSpeed + 1.0f) * shakeAmp;
        float noiseZ = sinf(shakeTimer * shakeSpeed + 2.3f) * shakeAmp;

        XMFLOAT3 shakenPos = basePosition;
        shakenPos.x += noiseX;
        shakenPos.z += noiseZ;

        SetPosition(shakenPos);
    }

    // 脱出
    if (tieTimer > selfRescueTimeInterval)
    {
        ReleasedTied();
    }

}

// 玉止め表示更新処理
void EnemyBase::UpdateTiedVisual()
{
    int showCount = 0;

    if (yarnSize == YarnSize::Small)
    {
        if (tieCount == 1)
        {
            showCount = 2; // 小は1回で2個
        }
    }
    else if (yarnSize == YarnSize::Big)
    {
        if (tieCount == 1)
        {
            showCount = 1;
            state = YarnState::Tying; // 玉止めの途中とする
        }
        else if (tieCount >= 2) showCount = 2;
    }

    for (int i = 0; i < tiedMeshes.size(); i++)
    {
        tiedMeshes[i]->SetIsVisible(i < showCount);
        tiedMeshes[i]->SetRelativeEulerRotationDirect({ 0.0f,180.0f,0.0f });
    }
}

// サイズ変更演出更新処理
void EnemyBase::UpdateSizeChanging(float deltaTime)
{
    sizeChangeTimer += deltaTime;

    // 点滅
    blinkTimer += deltaTime;
    if (blinkTimer >= blinkInterval)
    {
        blinkTimer = 0.0f;
        blinkOn = !blinkOn;
    }

    if (blinkOn)
    {
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };
    }
    else
    {
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,0.3f,0.3f,1 }; // 赤っぽく
    }

    // 終了
    if (sizeChangeTimer >= sizeChangeDuration)
    {
        isSizeChanging = false;

        // 色戻す
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

        // 実際のサイズ変更
        ChangeSize(pendingSize);
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
    else if (isRescuing && !isCutting)
    {
        float openMax = 50.0f;

        float t = rescueTimer / prepareTimeInterval;
        if (t > 1.0f) t = 1.0f;

        angle = openMax * t;
    }
    else if (isCutting)
    {
        scissorsCutTimer += deltaTime;
        float t = scissorsCutTimer / cutTimeInterval;

        if (t >= 1.0f)
        {
            t = 1.0f;
        }

        // 30 → 0 に補間
        angle = 50.0f * (1.0f - t);
    }
    else
    {
        angle = 0.0f;
    }

    scissorsFirstMeshComponent->SetRelativeEulerRotationDirect({ 0, angle, 0 });
    scissorsSecondMeshComponent->SetRelativeEulerRotationDirect({ 0, -angle, 0 });

}

// モデル選択関数
std::string EnemyBase::GetModelPath()
{
    switch (enemyType)
    {
    case YarnEnemyType::LongRangeAttack:
        return "./Data/TeamModels/Enemy/NeedleEnemy.glb";

    case YarnEnemyType::ChasePlayer:
        return (yarnSize == Big)
            ? "./Data/TeamModels/Enemy/YarnBigEnemyChase.glb"
            : "./Data/TeamModels/Enemy/YarnEnemyChase.glb";

    default:
        return (yarnSize == Big)
            ? "./Data/TeamModels/Enemy/YarnBigEnemy.glb"
            : "./Data/TeamModels/Enemy/YarnEnemy.glb";
    }
}

// 玉止めのモデル選択
void EnemyBase::GetTiedModelPath(std::string& leftTiedModelPath, std::string& rightTiedModelPath) const
{
    switch (enemyType)
    {
    case YarnEnemyType::LongRangeAttack:
        leftTiedModelPath = "./Data/TeamModels/Item/tiedModelLeftBigNeedle.glb";
        rightTiedModelPath = "./Data/TeamModels/Item/tiedModelRightBigNeedle.glb";
        break;
    default:
        if (yarnSize == Big)
        {
            leftTiedModelPath = "./Data/TeamModels/Item/tiedModelLeftBig.glb";
            rightTiedModelPath = "./Data/TeamModels/Item/tiedModelRightBig.glb";
        }
        else
        {
            leftTiedModelPath = "./Data/TeamModels/Item/tiedModelLeft.glb";
            rightTiedModelPath = "./Data/TeamModels/Item/tiedModelRight.glb";
        }
        break;
    }

}

// 当たり判定を作成
void EnemyBase::CreateCollisionComponent()
{
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
    //sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Bobbin, CollisionComponent::CollisionResponse::Block);
    sphereCollisionComponent->SetResponseToLayer(CollisionLayer::EnemyRedirect, CollisionComponent::CollisionResponse::Block);
    sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
    sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Bomb, CollisionComponent::CollisionResponse::Trigger);
    sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
    sphereCollisionComponent->Initialize();
    sphereCollisionComponent->SetOnHitCallback([this](CollisionComponent* self, CollisionComponent* other)
        {
            uint32_t mask = /*CollisionHelper::ToBit(CollisionLayer::Bobbin) |*/ CollisionHelper::ToBit(CollisionLayer::EnemyRedirect);
            if (!(other->GetCollisionLayer() & mask))
                return;

            auto dir = GetMoveDirection();

            auto myPos = GetPosition();
            auto otherPos = other->GetOwner()->GetPosition();

            float dx = myPos.x - otherPos.x;
            float dz = myPos.z - otherPos.z;

            // どっち方向の衝突が強いか
            if (abs(dx) > abs(dz))
            {
                // 横から当たった → X反転
                dir.x *= -1.0f;
            }
            else
            {
                // 縦から当たった → Z反転
                dir.z *= -1.0f;
            }

            SetMoveDirection(dir);
        });
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
    if (waitBeforeKnockback)
    {
        delayTimer += deltaTime;

        if (delayTimer >= delayBeforeKnockback)
        {
            waitBeforeKnockback = false;
            startKnockback = true; // ←ここで発火
            InputSystem::SetVibration(0.5f, 0.1f);
        }
    }

    if (startKnockback)
    {// 死亡したら
        // 上へ吹っ飛ぶ処理
        if (isKnockbackActive)
        {
            if (!popupScore)
            {
                // スコアデータを取得する
                auto data = GetScoreData();
                auto pos = GetPosition();
                // スコア処理　足されたスコアを取得する コンボ加算
                int addScore = ScoreSystem::ProcessHit(data, true);
                SpawnScorePopup(pos, addScore);
                popupScore = true;
            }

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
    else
    {
        // 常に白くする
        skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = 1.0f;
        // 発光強めると分かりやすい
        skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 2.0f;
        // 
        tieCount = GetNeedTiedCount();
    }
}

// 死亡した時に呼ぶ関数
void EnemyBase::CallDeath(bool hitByReflected)
{
    startKnockback = true;

    if (onDeath)
    {// WaveManagerにenemyCountを減らすように通知する
        onDeath();
    }

    // エフェクトを発生させる
    SpawnHitEffect(hitByReflected);

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

    float distance = hitByReflected ? tuning.knockbackDistanceNormal : tuning.knockbackDistanceDash;
    float height = hitByReflected ? tuning.knockbackHeightNormal : tuning.knockbackHeightDash;
    float duration = hitByReflected ? tuning.knockbackDurationNormal : tuning.knockbackDurationDash;
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

// サイズを変更演出を開始する
void EnemyBase::StartChangeSize(YarnSize newSize)
{
    if (yarnSize == newSize) return;

    isSizeChanging = true;
    sizeChangeTimer = 0.0f;
    blinkTimer = 0.0f;
    blinkOn = false;

    pendingSize = newSize;
}

// コインを生成する
void EnemyBase::SpawnCoin(DirectX::XMFLOAT3 pos)
{
    // コインを生成する
    Transform coinTr(pos, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto coin = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<ButtonCoinActor>("coin", coinTr);
    coin->StartPerform(false);
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
    if (attack) attack->Enter(this);
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

    selfBigRescueTimeInterval = 5.0f;// 大きい敵が自力脱出までかかる時間
    selfSmallRescueTimeInterval = 8.0f;// 小さい敵が自力脱出までかかる時間
}


void EnemyBase::SetUpVisual()
{
    std::string modelPath = GetModelPath();

    skeletalMeshComponent->SetModel(modelPath, false, true);
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };

    // 当たり判定を作成する
    CreateCollisionComponent();

    std::string leftTiedModelName;
    std::string rightTiedModelName;
    GetTiedModelPath(leftTiedModelName, rightTiedModelName);

    // 玉止めモデル
    auto tiedLeft = AddComponent<SkeletalMeshComponent>("tiedLeftMeshComponent", parentName);
    tiedLeft->SetModel(leftTiedModelName, false, true);
    tiedLeft->SetIsVisible(false);
    tiedLeft->SetIsCastShadow(false);
    tiedMeshes.push_back(tiedLeft);

    auto tiedRight = AddComponent<SkeletalMeshComponent>("tiedRightMeshComponent", parentName);
    tiedRight->SetModel(rightTiedModelName, false, true);
    tiedRight->SetIsVisible(false);
    tiedRight->SetIsCastShadow(false);
    tiedMeshes.push_back(tiedRight);


    switch (yarnSize)
    {
    case Small:
    {
        // スコアの設定
        scoreData = { 100,0 };
    }
    break;
    case Big:
    {
        // スコアの設定
        scoreData = { 200,0 };
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

    }
    break;
    }


    if (enemyType == YarnEnemyType::LongRangeAttack)
    {
        yarnSize = Big; // 見た目に合わせる
        scoreData = { 250,0 };
    }

    // 位置を更新　当たり判定が{0,0,0}にくるのを防ぐため
    UpdateAllComponentTransforms();

}


// 敵のサイズを変更する
void EnemyBase::ChangeSize(EnemyBase::YarnSize newSize)
{
    if (yarnSize == newSize)
    {// 現在のサイズと同じだったら
        Logger::Log(U8(""));
        return;
    }

    if (IsDead())
    {// 死亡していたら
        return;
    }

    yarnSize = newSize;

    // 玉止め解除
    ReleasedTied();

#if 1
    // 既存コンポーネントを削除
    RequestDestroyComponent("sphereComponent");
    RequestDestroyComponent("tiedLeftMeshComponent");
    RequestDestroyComponent("tiedRightMeshComponent");
    tiedMeshes.clear();
#endif // 0


    if (scissorsFirstMeshComponent.get() && scissorsSecondMeshComponent.get())
    {// 大きくなる敵がハサミ持ちの場合場所を調整する
        scissorsFirstMeshComponent->SetRelativeLocationDirect({ 0, 1.3f, 0 });
        scissorsFirstMeshComponent->SetRelativeScaleDirect({ 1.2f,1.2f,1.2f });

        scissorsSecondMeshComponent->SetRelativeLocationDirect({ 0, 1.3f, 0 });
        scissorsSecondMeshComponent->SetRelativeScaleDirect({ 1.2f,1.2f,1.2f });
    }


    // 見た目・当たり判定を再生成
    SetUpVisual();
    UpdateAllComponentTransforms();
}