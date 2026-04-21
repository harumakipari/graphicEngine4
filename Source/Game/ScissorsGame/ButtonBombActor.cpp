#include "pch.h"

#include "ButtonBombActor.h"

#include "ScissorsPlayer1.h"
#include "YarnEnemyActor.h"
#include "Engine/Scene/SceneBase.h"

void ButtonBombActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/enemy.glb", false, true);

    // 当たり判定
    {
        std::shared_ptr<SphereComponent> sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        float radius = size.x * 0.5f;
        float height = size.y;
        float mass = 60.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(mass);
        sphereComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereComponent->SetLayer(CollisionLayer::Enemy);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetCollisionOffsetY(height * 0.5f);
        sphereComponent->Initialize();
    }

}

void ButtonBombActor::Update(float deltaTime)
{

}

// 爆発処理
void ButtonBombActor::Explode()
{
    float radius = 3.0f;

    for (auto enemy : GetOwnerScene()->GetActorManager()->GetActorsOfType<ScissorsGameEnemyBase>())
    {
        if (!enemy) continue;

        float dist = MathHelper::Distance(enemy->GetPosition(), GetPosition());

        if (dist < radius)
        {
            enemy->TakeDamage(2, false);
        }
    }

    // プレイヤーにもダメージ
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (player)
    {
        float dist = MathHelper::Distance(player->GetPosition(), GetPosition());
        if (dist < radius)
        {
            player->TakeDamage(20);
        }
    }
}
