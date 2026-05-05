#include "pch.h"
#include "RabbitBossEnemy.h"

#include "BossSpawner.h"
#include "EnemyBase.h"
#include "ScissorsPlayer1.h"
#include "RabbitBossState.h"
#include "ScissorsPlayerStateDerived.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void RabbitBossEnemyActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/BossEnemy.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    //skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameBossPS";

    // 当たり判定
    {
        collisionBoxComponent = this->AddComponent<BoxComponent>("boxComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        collisionBoxComponent->SetBoxExtent(size);
        collisionBoxComponent->SetStatic(true);
        collisionBoxComponent->SetMass(0.0f);
        collisionBoxComponent->SetLayer(CollisionLayer::Boss);
        collisionBoxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        collisionBoxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        collisionBoxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        collisionBoxComponent->SetCollisionOffsetY(size.y * 0.5f);
        collisionBoxComponent->Initialize();

#if 0
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
        height = size.y;
        sphereCollisionComponent->SetStatic(true);
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(0.0f);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Boss);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
#endif // 0
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);
    rotationComponent->SetDirection({ 0,0,-1 });
    // Hpの初期化
    maxHp = 500;
    hp = maxHp;


    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 3000,0 };

    // ボスのHPのUI
    {
        auto uiManager = GetOwnerScene()->GetUIManager();
        DirectX::XMFLOAT2 gaugeSize = { 400.0f,50.0f };

        // ボスのHPゲージのフレームスプライト描画コンポーネントを追加
        gaugeFrameBackComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/bar_back.png", "bar_back_ui");
        gaugeFrameBackComponent->SetWorldPosition({ 67, 965 });
        gaugeFrameBackComponent->SetScale({ 1.0f, 1.0f });
        gaugeFrameBackComponent->SetSize(gaugeSize);
        gaugeFrameBackComponent->zOrder = 10;
        //gaugeFrameBackComponent->SetPivot({ 0.0f,0.5f });
        gaugeFrameBackComponent->SetColor(CoreColor::White);
        uiManager->Add(gaugeFrameBackComponent);

        gaugeUi = std::make_shared<UIGaugeComponent>("./Data/Textures/ScissorsUI/bar_line.png", "./Data/Textures/ScissorsUI/bar.png", "bossGauge");
        gaugeUi->SetWorldPosition({ 50, 300 });
        gaugeUi->zOrder = 15;
        gaugeUi->SetSize(gaugeSize);

        uiManager->Add(gaugeUi);
    }

    // ステートマシンを作成
    stateMachine_ = std::make_shared<StateMachine>();
    stateMachine_->RegisterState(std::make_unique<RabbitBossIdleState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossAttackSelectState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossAttackWarpPreviewState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossAttackWarpState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossAttackBuffPreviewState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossAttackBuffState>(this));
    stateMachine_->RegisterState(std::make_unique<RabbitBossStunState>(this));

    // ステートマシンを character に追加
    this->SetStateMachine(stateMachine_);
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");


    // 出現ポイント
    spawnPoints =
    {
        {2,0,11},
        {11,0,11},
        {11,0,20,},
        {11,0,2},
        {20,0,11}
    };
}

void RabbitBossEnemyActor::Update(float deltaTime)
{
    Character::Update(deltaTime);

    DirectX::XMFLOAT3 pos = GetPosition();

    // HPバーの処理
    {
        DirectX::XMFLOAT2 uiPos = WorldToUI(pos);
        float hpGauge = static_cast<float>(hp);
        float hpGaugeMax = static_cast<float>(maxHp);
        uiPos.x += gaugeUiOffset.x;
        uiPos.y += gaugeUiOffset.y;
        if (gaugeUi)
        {
            gaugeUi->SetValue(hpGauge, hpGaugeMax);
            gaugeUi->SetWorldPosition({ uiPos.x, uiPos.y });
            //gaugeUi->SetColor({ color.x,color.y,color.z,color.w });
            gaugeUi->SetGaugeOffset(gaugeFrameOffset);

            gaugeFrameBackComponent->SetWorldPosition({ uiPos.x, uiPos.y });
        }
    }

    // 沈む処理
    if (isDiving)
    {
        diveOffsetY -= diveSpeed * deltaTime;

        if (diveOffsetY <= maxDiveDepth)
        {
            diveOffsetY = maxDiveDepth;
            isDiving = false;
        }

        ApplyDiveOffset();
    }

    // 出現処理
    if (isEmerging)
    {
        diveOffsetY += diveSpeed * deltaTime;

        if (diveOffsetY >= 0.0f)
        {
            diveOffsetY = 0.0f;
            isEmerging = false;
        }

        ApplyDiveOffset();
    }

    // 出現範囲のデバック描画
    DebugRender::DrawSphere(pos, spawnAttackRange, { 1,0,0.5f,1 }, 0, true);
}

void RabbitBossEnemyActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI

    ImGui::DragFloat2(U8("ゲージのオフセット値"), &gaugeUiOffset.x, 2.0f);
    ImGui::DragFloat2(U8("ゲージフレームのオフセット値"), &gaugeFrameOffset.x, 2.0f);
    ImGui::DragFloat(U8("出現攻撃範囲"), &spawnAttackRange, 0.5f, 0.0f, 10.0f);

#endif
}

// スタン状態かどうか
bool RabbitBossEnemyActor::IsStunned()
{
    return (GetStateMachine()->GetStateName() == "Stun");
}


// ダメージ処理計算
float RabbitBossEnemyActor::ComputeDamage(const BossDamageContext& damageContext)
{
    float damage = damageContext.baseDamage;

    if (damageContext.isBossStunned) // 20ダメージ
        damage *= 2.0f;

    if (damageContext.killedEnemyBeforeHitCount)    // 
    {
        float multiple = 1.0f + 0.2f * damageContext.killedEnemyBeforeHitCount;
        damage *= multiple;
    }


    return damage;
}

// 半透明の処理
void RabbitBossEnemyActor::SetRenderOpacity(float opacity)
{
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor.w = opacity;
}

void RabbitBossEnemyActor::OnTied()
{
    if (GetStateMachine()->GetStateName() == "Warp")
    {// ワープ中は 
        return;// スタンしない
    }
    // スタン状態に入る
    EnterStun();
}


// ランダムに大きい敵に変更する処理
void RabbitBossEnemyActor::EnlargeRandomEnemies(int count)
{
    std::vector<std::shared_ptr<EnemyBase>> candidates;

    auto bossSpawner = GetOwnerScene()->GetActorManager()->GetActorOfType<BossSpawner>();

    if (!bossSpawner)
    {
        Logger::Warning(U8("bossSpawner が nullptr　です！"));
        return;
    }

    // Smallだけ集める
    for (auto& w : bossSpawner->aliveEnemies)
    {
        if (auto e = w.lock())
        {
            if (!e->IsDead() &&
                /*e->GetState() == EnemyBase::YarnState::Active &&*/
                e->GetYarnSize() == Small) // Small判定
            {
                candidates.push_back(e);
            }
        }
    }

    if (candidates.empty()) return;

    // シャッフル
    std::shuffle(candidates.begin(), candidates.end(), std::mt19937(std::random_device{}()));

    int changeCount = std::min<int>(count, static_cast<int>(candidates.size()));

    for (int i = 0; i < changeCount; i++)
    {
        candidates[i]->StartChangeSize(EnemyBase::Big);
    }
}


void RabbitBossEnemyActor::CreteDamageZone()
{
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        Logger::Warning(U8("playerがnullptrです！"));
    }

    DirectX::XMFLOAT3 playerPos = player->GetPosition();
    playerPos.y = 0.0f;
    DirectX::XMFLOAT3 enemyPos = GetPosition();
    enemyPos.y = 0.0f;

    if (MathHelper::Distance(playerPos, enemyPos) < spawnAttackRange)
    {
        player->TakeDamage(3);
    }
}

// スタン状態に入る
void RabbitBossEnemyActor::EnterStun()
{
    GetStateMachine()->ChangeState("Stun");
    Logger::Log(U8("ボスがスタンした！"));
}

// 爆弾を生成する
void RabbitBossEnemyActor::SpawnButtonBombs()
{
    const float offset = 2.0f;
    auto pos = GetPosition();

    std::vector<DirectX::XMFLOAT3> offsets =
    {
        { offset,0,0},
        {-offset,0,0},
        {0,0,offset},
        {0,0,-offset}
    };

    for (auto& o : offsets)
    {
        auto bombPos = pos;
        bombPos.x += o.x;
        bombPos.z += o.z;

        // TODO: ButtonBomb生成
        // CreateActor<ButtonBomb>(bombPos);
    }
}

// Ｙ座標を下げる処理
void RabbitBossEnemyActor::ApplyDiveOffset()
{
    auto pos = GetPosition();
    pos.y = 0.0f + diveOffsetY;
    SetPosition(pos);
}