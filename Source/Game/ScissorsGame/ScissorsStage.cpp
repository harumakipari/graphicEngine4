#include "pch.h"

#include "ScissorsStage.h"

#include "Engine/Scene/SceneBase.h"
#include "Game/Actors/Player/Player.h"

void ScissorsStage::Initialize(const Transform& transform)
{
    std::string parentName = "ScissorsStage";

    staticMeshComponent = AddComponent<StaticMeshComponent>(parentName);
    staticMeshComponent->SetModel("./Data/TeamModels/Stage/ScissorsStage.glb", false, false);

    // 床の当たり判定用のボックスコリジョンコンポーネント
    std::shared_ptr<BoxComponent> boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(40.0f, 0.2f, 40.0f));
    boxComponent->SetRelativeLocationDirect({ 0.0f,-0.2f,0.0f });
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldStatic);
    boxComponent->Initialize();


    //staticMeshComponent->SetModel("./Data/TeamModels/Stage/Stage.gltf", false, false);
}

void ScissorsStage::Update(float deltaTime)
{

}