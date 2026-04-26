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
    boxComponent->SetLayer(CollisionLayer::Floor);
    boxComponent->Initialize();

    {
        // 左壁の当たり判定用のボックスコリジョンコンポーネント
        std::shared_ptr<BoxComponent> wallComponent = this->AddComponent<class BoxComponent>("wallLeftComponent", parentName);
        wallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.0f, 5.0f, 20.0f));
        wallComponent->SetRelativeLocationDirect({ 1.0f,-0.f,-4.0f });
        wallComponent->SetStatic(true);
        wallComponent->SetLayer(CollisionLayer::Wall);
        wallComponent->Initialize();
    }

    // 壁の当たり判定用のボックスコリジョンコンポーネント
    {
        std::shared_ptr<BoxComponent> wallComponent = this->AddComponent<class BoxComponent>("wallRightComponent", parentName);
        wallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(1.0f, 5.0f, 20.0f));
        wallComponent->SetRelativeLocationDirect({ -21.0f,-0.f,-4.0f });
        wallComponent->SetStatic(true);
        wallComponent->SetLayer(CollisionLayer::Wall);
        wallComponent->Initialize();
    }

    // 壁の当たり判定用のボックスコリジョンコンポーネント
    {
        std::shared_ptr<BoxComponent> wallComponent = this->AddComponent<class BoxComponent>("wallFrontComponent", parentName);
        wallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(20.0f, 5.0f, 1.0f));
        wallComponent->SetRelativeLocationDirect({ -3.0f,-0.f,1.7f });
        wallComponent->SetStatic(true);
        wallComponent->SetLayer(CollisionLayer::Wall);
        wallComponent->Initialize();
    }

    // 壁の当たり判定用のボックスコリジョンコンポーネント
    {
        std::shared_ptr<BoxComponent> wallComponent = this->AddComponent<class BoxComponent>("wallBackComponent", parentName);
        wallComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(20.0f, 5.0f, 1.0f));
        wallComponent->SetRelativeLocationDirect({ -3.0f,-0.f,-21.7f });
        wallComponent->SetStatic(true);
        wallComponent->SetLayer(CollisionLayer::Wall);
        wallComponent->Initialize();
    }
    //staticMeshComponent->SetModel("./Data/TeamModels/Stage/Stage.gltf", false, false);
}

void ScissorsStage::Update(float deltaTime)
{

}