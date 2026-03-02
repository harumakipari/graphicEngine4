#include "pch.h"
#include "DarkStageBarrelActor.h"

void DarkStageBarrelActor::Initialize(const Transform& transform)
{
    std::string parentName = "barrelMesh";

    // ’M‚Ìƒ‚ƒfƒ‹‚ğ’Ç‰Á
    barrelMeshComponent = this->AddComponent<SkeletalMeshComponent>(parentName);
    barrelMeshComponent->SetModel("./Data/Models/DarkStageAssets/Candelabra/Candelabra.gltf");
    barrelMeshComponent->SetIsCastShadow(false);    // ‰e‚ğ—‚Æ‚³‚È‚¢‚æ‚¤‚É‚·‚é

    // ’M‚Ì‚ª‚ê‚«‚Ég—p‚·‚éƒ‚ƒfƒ‹
    auto barrelConvexMeshComponent = AddComponent<SkeletalMeshComponent>("barrelConvexMesh", parentName);
    barrelConvexMeshComponent->SetModel("./Data/Models/DarkStageAssets/Barrel_Convex1/Barrel_Convex1.gltf", true);
    barrelConvexMeshComponent->SetIsVisible(false);

    // Å‰‚Ì‰ó‚ê‚é‘O‚Ì” ‚Ì“–‚½‚è”»’è
    auto boxComponent = AddComponent<BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = barrelMeshComponent->model->GetModelSize();
    boxComponent->SetBoxExtent(size);
    float height = size.y * 0.5f;
    boxComponent->SetCollisionOffsetY(height);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldStatic);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

    // ’M‚ÌŠ¢âI
    auto convexComponent = AddComponent<ConvexCollisionComponent>("convexComponent", parentName);
    convexComponent->SetLayer(CollisionLayer::Convex);
    convexComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetActive(false);
    convexComponent->CreateConvexMeshFromModel(barrelConvexMeshComponent.get());
}
