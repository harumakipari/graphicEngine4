#include "pch.h"
#include "RabbitBossEnemy.h"

#include "EnemyBase.h"
#include "ScissorsPlayer1.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void RabbitBossEnemyActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/BossEnemy.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // emissionPowerの値を大きくして、自己発光の強さを上げてみる
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.brightness = 5.0f;
    skeletalMeshComponent->plusAlphaCBuffer->data.saturation = 1.4f;

    // 当たり判定
    {
        auto boxComponent= this->AddComponent<BoxComponent>("boxComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        boxComponent->SetBoxExtent(size);
        boxComponent->SetStatic(true);
        boxComponent->SetMass(0.0f);
        boxComponent->SetLayer(CollisionLayer::Boss);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetCollisionOffsetY(size.y * 0.5f);
        boxComponent->Initialize();

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
    maxHp = 10;
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 3000,0 };

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
    attackTimer += deltaTime;
#if 0

    if (attackTimer > attackTimeInterval)
    {
        EnlargeRandomEnemies(3); // 3体強化
        Logger::Log(U8("ボスの攻撃"));
        attackTimer = 0.0f;
    }

#endif // 0

    //DebugRender::DrawSphere(center, currentRadius, { 1,0,0.5f,1 }, 0, true);
}

void RabbitBossEnemyActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat(U8("出現攻撃範囲"), &spawnAttackRange, 0.5f, 0.0f, 10.0f);
#endif
}


// ダメージ処理計算
float RabbitBossEnemyActor::ComputeDamage(const BossDamageContext& damageContext)
{
    float damage = damageContext.baseDamage;

    if (damageContext.isKilledEnemyBeforeHit)
        damage *= 2.0f;

    if (damageContext.isBossStunned)
        damage *= 3.0f;

    return damage;
}

// ランダムに大きい敵に変更する処理
void RabbitBossEnemyActor::EnlargeRandomEnemies(int count)
{
    std::vector<std::shared_ptr<EnemyBase>> candidates;

    auto waveManager = GetOwnerScene()->GetActorManager()->GetActorOfType<WaveManager>();

    // Smallだけ集める
    for (auto& w : waveManager->aliveEnemies)
    {
        if (auto e = w.lock())
        {
            if (!e->IsDead() &&
                e->GetState() == EnemyBase::YarnState::Active &&
                e->GetNeedTiedCount() == 1) // Small判定
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
        candidates[i]->ChangeSize(EnemyBase::Big);
    }
}


void RabbitBossEnemyActor::CreteDamageZone(const DirectX::XMFLOAT3& pos)
{
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        Logger::Warning(U8("playerがnullptrです！"));
    }

    if (MathHelper::Distance(player->GetPosition(), pos) < spawnAttackRange)
    {
        player->TakeDamage(2);
    }
}

void RabbitBossEnemyActor::SpawnRandomPoint()
{
    if (spawnPoints.empty()) return;

    int index = rand() % spawnPoints.size();
    auto pos = spawnPoints[index];

    SetPosition(pos);

    // 出現ダメージ
    CreteDamageZone(pos);
}
