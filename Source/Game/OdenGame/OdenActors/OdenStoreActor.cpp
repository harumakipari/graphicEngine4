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


    auto stageModel = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    stageModel->SetModel("./Data/Models/Oden_Store/Oden_GameStage.gltf", false);
    //stageModel->SetRelativeScaleDirect({ -1.0f,1.0f,1.0f });
    stageModel->SetRelativeLocationDirect({ 6.0f,0.0f,2.0f });

    storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Store/Oden_frame.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    storeModelComponent->SetRelativeLocationDirect({ 6.0f,0.0f,2.0f });
}


// –ˆƒtƒŒ[ƒ€ŒÄ‚Î‚ê‚é (0 ~ 1)
void OdenStoreActor::OnBeatPhase(float phase)
{
    float pulse = sinf(phase * DirectX::XM_2PI);
    float scale = 1.0f + pulse * 0.03f;

    storeModelComponent->SetRelativeScaleDirect({
        -1.0f * scale,
         1.0f * scale,
        -1.0f * scale});
}