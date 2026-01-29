#include "pch.h"
#include "OdenResultStageActor.h"

void OdenResultStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "RootComponent";

#if 0
    storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Yatai.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    storeModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    //auto groundModelComponent = AddComponent<StaticMeshComponent>("Oden_Ground_Model", parentName);
    //groundModelComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Title_Ground.gltf", false);
    //groundModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    //groundModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_under");
        DirectX::XMFLOAT3 size = { 80.0f,1.0f,80.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
    }

    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_oku");
        DirectX::XMFLOAT3 size = { 80.0f,40.0f,1.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
    }

    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_left");
        DirectX::XMFLOAT3 size = { 80.0f, 40.0f, 1.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
        boxComponent->SetRelativeLocationDirect({ -25.5f,0.0f,0.0f });
        boxComponent->SetRelativeEulerRotationDirect({ 0.0f,90.0f,0.0f });
    }

    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_right");
        DirectX::XMFLOAT3 size = { 80.0f,1.0f,80.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
        boxComponent->SetRelativeLocationDirect({ 27.2f,1.4f,0.0f });
        boxComponent->SetRelativeEulerRotationDirect({ 0.0f,0.0f,90.0f });
    }

#if 1
    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_temae");
        DirectX::XMFLOAT3 size = { 80.0f,1.0f,80.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
        //boxComponent->SetRelativeLocationDirect({ 2.0f,-2.5f,5.0f });
        boxComponent->SetRelativeLocationDirect({ 2.0f,-2.5f,-6.0f });
        boxComponent->SetRelativeEulerRotationDirect({ 90.0f,0.0f,0.0f });
    }

#endif // 1
#else
    std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->AddComponent<StaticMeshComponent>("staticMeshComponent", parentName);
    staticMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Pot.gltf", true);
    //staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
    //staticMeshComponent->SetRelativeLocationDirect({ 16.0f,0.0f,-4.0f });

    // 下
    {
        auto boxComponent = AddComponent<BoxComponent>("boxComponent_under");
        DirectX::XMFLOAT3 size = { 160.0f,1.0f,160.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(0.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
        boxComponent->SetRelativeLocationDirect({ 0.0f,1.8f,0.0f });
        //boxComponent->DisableCollision();
    }


    // 左
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxMesh_left", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Left.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        //// メッシュ
        //std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_left", "boxMesh_left");
        //triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        //triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        //triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
        //triangleMeshComponent->DisableCollision();

        auto boxComponent = AddComponent<BoxComponent>("boxComponent_left", "boxMesh_left");
        DirectX::XMFLOAT3 size = { 10.0f,1000.0f,10.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(0.0f);
        boxComponent->SetCollisionOffsetY(size.y * 0.5f);
        boxComponent->SetPhysicsMaterial(PhysicsMaterialType::Wall);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetRelativeLocationDirect({ -19.0f,0.0f,0.0f });
        boxComponent->Initialize();
    }

    // 右
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxMesh_right", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Right.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        // メッシュ
        //std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_right", "boxMesh_right");
        //triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        //triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        //triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
        //triangleMeshComponent->DisableCollision();

        rightBoxComponent = AddComponent<BoxComponent>("boxComponent_right", "boxMesh_right");
        DirectX::XMFLOAT3 size = { 10.0f,1000.0f,10.0f };
        rightBoxComponent->SetBoxExtent(size);
        rightBoxComponent->SetMass(0.0f);
        rightBoxComponent->SetCollisionOffsetY(size.y * 0.5f);
        rightBoxComponent->SetPhysicsMaterial(PhysicsMaterialType::Wall);
        rightBoxComponent->SetLayer(CollisionLayer::WorldStatic);
        rightBoxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        rightBoxComponent->SetRelativeLocationDirect({ 19.0f,0.0f,0.0f });
        rightBoxComponent->Initialize();

        rightAfterBoxComponent = AddComponent<BoxComponent>("boxComponent_rightAfter", "boxMesh_right");
        DirectX::XMFLOAT3 sizeAfter = { 10.0f,38.0f,10.0f };
        rightAfterBoxComponent->SetBoxExtent(sizeAfter);
        rightAfterBoxComponent->SetMass(0.0f);
        rightAfterBoxComponent->SetCollisionOffsetY(sizeAfter.y * 0.5f);
        rightAfterBoxComponent->SetPhysicsMaterial(PhysicsMaterialType::Wall);
        rightAfterBoxComponent->SetLayer(CollisionLayer::WorldStatic);
        rightAfterBoxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        rightAfterBoxComponent->SetRelativeLocationDirect({ 19.0f,0.0f,0.0f });
        rightAfterBoxComponent->Initialize();
        rightAfterBoxComponent->DisableCollision();
    }

    // 手前
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxMesh_front", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Front.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        //// メッシュ
        //std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_front", "boxMesh_front");
        //triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        //triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        //triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
        //triangleMeshComponent->DisableCollision();

        auto boxComponent = AddComponent<BoxComponent>("boxComponent_front", "boxMesh_front");
        DirectX::XMFLOAT3 size = { 30.0f,1000.0f,5.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(0.0f);
        boxComponent->SetCollisionOffsetY(size.y * 0.5f);
        boxComponent->SetPhysicsMaterial(PhysicsMaterialType::Wall);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,5.0f });
        boxComponent->Initialize();
    }

    // 奥
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxMesh_back", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Back.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        // メッシュ
        //std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_back", "boxMesh_back");
        //triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        //triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        //triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
        //triangleMeshComponent->DisableCollision();

        auto boxComponent = AddComponent<BoxComponent>("boxComponent_back", "boxMesh_back");
        DirectX::XMFLOAT3 size = { 30.0f,1000.0f,5.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(0.0f);
        boxComponent->SetCollisionOffsetY(size.y * 0.5f);
        boxComponent->SetPhysicsMaterial(PhysicsMaterialType::Wall);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,-5.4f });
        boxComponent->Initialize();


    }

#endif // 0

    // 汁のモデル
    soupModelComponent = AddComponent<StaticMeshComponent>("Oden_Soup_Model", parentName);
    soupModelComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Soup.gltf", false);
    soupModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    soupModelComponent->overrideForwardPipelineName = "OdenSoupSurfaceMesh";
    soupModelComponent->overrideDeferredPipelineName = "OdenSoupSurfaceMesh";
    soupModelComponent->SetPriority(-10);

}

void OdenResultStageActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("当たり判定を変更")))
    {
        rightBoxComponent->DisableCollision();
        rightAfterBoxComponent->EnableCollision();
    }

#endif
}

// 当たり判定を切り替える
void OdenResultStageActor::SwitchCollision()
{
    
}