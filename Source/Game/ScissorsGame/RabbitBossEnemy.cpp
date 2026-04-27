#include "pch.h"
#include "RabbitBossEnemy.h"

#include "EnemyBase.h"
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
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
        height = size.y;
        sphereCollisionComponent->SetStatic(true);
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(0.0f);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Enemy);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };
}

void RabbitBossEnemyActor::Update(float deltaTime)
{
    attackTimer += deltaTime;

    if (attackTimer > attackTimeInterval)
    {
        EnlargeRandomEnemies(3); // 3体強化
        attackTimer = 0.0f;
    }
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


