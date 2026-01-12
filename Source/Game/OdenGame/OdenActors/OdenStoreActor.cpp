#include "pch.h"
#include "OdenStoreActor.h"

void OdenStoreActor::Initialize(const Transform& transform)
{
    std::string parentName = "RootComponent";

    auto boxComponent = AddComponent<BoxComponent>("Oden_BoxComponent", parentName);
    DirectX::XMFLOAT3 size = { 40.0f,1.0f,40.0f };
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::WorldStatic);
    //boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetRelativeLocationDirect({ 6.0f,0.0f,6.0f });
    boxComponent->Initialize();

    auto storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Store/Oden_frame.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });


}
