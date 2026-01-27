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
#endif // 0

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
        DirectX::XMFLOAT3 size ={ 80.0f, 40.0f, 1.0f };
        boxComponent->SetBoxExtent(size);
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::WorldStatic);
        boxComponent->SetResponseToLayer(CollisionLayer::OdenHoverTarget, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();
        boxComponent->SetRelativeLocationDirect({ -20.5f,0.0f,0.0f });
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
}





