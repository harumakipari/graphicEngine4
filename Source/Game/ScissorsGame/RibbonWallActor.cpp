#include "pch.h"
#include "RibbonWallActor.h"

void RibbonWallActor::Initialize(const Transform& transform)
{
    std::string parentName = "RibbonWallActor";

    wallCollisionComponent = AddComponent<SphereComponent>(parentName);
    wallCollisionComponent->SetRadius(0.4f); // ← 少し大きめが重要
    wallCollisionComponent->SetLayer(CollisionLayer::RibbonWall);
    wallCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    wallCollisionComponent->SetStatic(true); // 壁だから動かせないようにする
    wallCollisionComponent->Initialize();

    auto skeletalMeshComponent = AddComponent<SkeletalMeshComponent>("mesh", parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/Wall.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // emissionPowerの値を大きくして、自己発光の強さを上げてみる
    skeletalMeshComponent->overrideDeferredPipelineName = "deferredFightStage";
    skeletalMeshComponent->plusAlphaCBuffer->data.brightness = 5.0f;
    skeletalMeshComponent->plusAlphaCBuffer->data.saturation = 1.4f;

}

// 壁を壊す
void RibbonWallActor::Break()
{
    MarkPendingKill();
}