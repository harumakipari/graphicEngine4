#include "pch.h"
#include "RibbonWallActor.h"

void RibbonWallActor::Initialize(const Transform& transform)
{
    std::string parentName = "RibbonWallActor";

    wallCollisionComponent = AddComponent<SphereComponent>(parentName);
    wallCollisionComponent->SetRadius(0.4f); // © ­‚µ‘å‚«‚ß‚ªd—v
    wallCollisionComponent->SetLayer(CollisionLayer::Convex);
    wallCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    wallCollisionComponent->SetStatic(true); // •Ç‚¾‚©‚ç“®‚©‚¹‚È‚¢‚æ‚¤‚É‚·‚é
    wallCollisionComponent->Initialize();
    

#if 0
    auto skeletalMeshComponent = AddComponent<SkeletalMeshComponent>("mesh", parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/Wall.glb", false, true);
#endif // 0

    
}

// •Ç‚ð‰ó‚·
void RibbonWallActor::Break()
{
    if (wallCollisionComponent)
    {
        wallCollisionComponent->DisableCollision();
    }
    MarkPendingKill();
}