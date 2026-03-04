#include "pch.h"
#include "DarkStageBarrelActor.h"

void DarkStageBarrelActor::Initialize(const Transform& transform)
{
    std::string parentName = "barrelMesh";

    // 樽のモデルを追加
    barrelMeshComponent = this->AddComponent<SkeletalMeshComponent>(parentName);
    barrelMeshComponent->SetModel("./Data/Models/DarkStageAssets/Barrel/SM_Barrel_01.gltf");
    barrelMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする

    // 樽のがれきに使用するモデル
    auto barrelConvexMeshComponent = AddComponent<SkeletalMeshComponent>("barrelConvexMesh", parentName);
    barrelConvexMeshComponent->SetModel("./Data/Models/DarkStageAssets/Barrel_Convex1/Barrel_Convex1.gltf", true);
    barrelConvexMeshComponent->SetIsVisible(false);

    // 最初の壊れる前の箱の当たり判定
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

    // 樽の瓦礫
    auto convexComponent = AddComponent<ConvexCollisionComponent>("convexComponent", parentName);
    convexComponent->SetLayer(CollisionLayer::Convex);
    convexComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    convexComponent->SetActive(false);
    convexComponent->CreateConvexMeshFromModel(barrelConvexMeshComponent.get());
}

void DarkStageBarrelActor::Update(float deltaTime)
{
#if 0
    // 元々の箱の当たり判定を消す
    boxComponent->DisableCollision();
    this->ScheduleDestroyComponentByName("boxComponent");

    //this->ScheduleDestroyComponentByName("boxComponent");
    //this->DestroyComponentByName("boxComponent");
    //// ビームを消す
    //beam->SetValid(false);
    // 瓦礫を当たり判定に入れる
    if (convexComponent)
    {
        convexComponent->AddToScene(); // ここで physx の scene に追加する　ここまでは物理演算の考慮に入れたくないから
        convexComponent->SetKinematic(false);
        convexComponent->SetActive(true);
    }
#endif // 0
}

void DarkStageBarrelActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    //if (ImGui::TreeNode("DarkStageBarrelActor"))
    //{
    //    ImGui::Text("This is a barrel that can be destroyed.");
    //    ImGui::TreePop();
    //}
#endif
}