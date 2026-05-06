#include "pch.h"
#include "ItemHeartActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void ItemHeartActor::Initialize(const Transform& transform)
{
    std::string parentName = "ItemHeartActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/ItemHeartModel.gltf", false, true);


    auto boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    float  mass = 0.0f;
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,size.y * 0.5f ,0.0f });
    boxComponent->SetMass(mass);
    boxComponent->SetLayer(CollisionLayer::Item);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->Initialize();
    boxComponent->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            uint32_t mask = CollisionHelper::ToBit(CollisionLayer::PlayerWeapon) | CollisionHelper::ToBit(CollisionLayer::Player);
            if (other->GetCollisionLayer() & mask)
            {
                UseItem();
            }
        }
    );
}

void ItemHeartActor::Update(float deltaTime)
{

}

void ItemHeartActor::DrawImGuiDetails()
{

}

// アイテムを使用する
void ItemHeartActor::UseItem()
{
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        return;
    }
    player->RecoverHp(2);

    MarkPendingKill();
}


