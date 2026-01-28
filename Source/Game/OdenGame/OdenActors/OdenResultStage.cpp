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
        DirectX::XMFLOAT3 size = { 80.0f,1.0f,80.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
    }


    // 左
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxComponent_left", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Left.gltf", true);
        //boxMeshComponent->SetIsVisible(false);
        // メッシュ
        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_left", "boxComponent_left");
        triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
    }

    // 右
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxComponent_right", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Right.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        // メッシュ
        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_right", "boxComponent_right");
        triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
    }

    // 手前
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxComponent_front", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Front.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        // メッシュ
        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_front", "boxComponent_front");
        triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
    }

    // 奥
    {
        std::shared_ptr<StaticMeshComponent> boxMeshComponent = this->AddComponent<StaticMeshComponent>("boxComponent_back", parentName);
        boxMeshComponent->SetModel("./Data/Models/Oden_Result_Stage/Oden_Result_Collider_Back.gltf", true);
        boxMeshComponent->SetIsVisible(false);
        // メッシュ
        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent_back", "boxComponent_back");
        triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        triangleMeshComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        triangleMeshComponent->CreateConvexMeshFromModel(boxMeshComponent.get());
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





