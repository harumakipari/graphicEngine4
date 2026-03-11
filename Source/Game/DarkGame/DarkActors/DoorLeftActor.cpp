#include "pch.h"
#include "DoorLeftActor.h"

void DoorLeftActor::Initialize(const Transform& transform)
{
    std::string parentName = "Door_Left";

    // ドアのメッシュコンポーネントを追加
    leftMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    leftMeshComponent->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
    // ドアのサイズを取得
    DirectX::XMFLOAT3 size = leftMeshComponent->GetModelSize();

#if 1
    // ドアの当たり判定用のコリジョンコンポーネントを追加
    std::shared_ptr<BoxComponent> boxComponent = AddComponent<BoxComponent>("DoorCollision", parentName);
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,-1.1f });
    boxComponent->SetCollisionOffsetY(size.y * 0.5f);
    boxComponent->SetCollisionOffsetX(-size.x * 0.5f);
    boxComponent->SetCollisionOffsetZ(-size.z * 0.5f);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldProps);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

#endif // 0
    //// ドアにトリガー用のコリジョンコンポーネントを追加
    //auto interactTrigger = AddComponent<BoxComponent>("DoorTrigger", parentName);
    //DirectX::XMFLOAT3 triggerSize = { size.x * 1.2f ,size.y * 1.2f , size.z * 1.2f };// 少し大きくする
    //interactTrigger->SetBoxExtent(triggerSize);
    //interactTrigger->SetCollisionOffsetY(triggerSize.y * 0.5f);
    //interactTrigger->SetCollisionOffsetX(-triggerSize.x * 0.5f);
    //interactTrigger->SetCollisionOffsetZ(-triggerSize.z * 0.5f);
    //interactTrigger->SetLayer(CollisionLayer::Interactable);
    //interactTrigger->SetResponseToLayer(
    //    CollisionLayer::Player,
    //    CollisionComponent::CollisionResponse::Trigger
    //);
    //interactTrigger->SetTrigger(true);
    //interactTrigger->Initialize();

}

void DoorLeftActor::Update(float deltaTime)
{
    switch (doorState)
    {
    case DoorLeftActor::DoorState::Closed:

        break;
    case DoorLeftActor::DoorState::Opening:
    {
        DirectX::XMFLOAT4 orientation = GetQuaternionRotation();
        DirectX::XMVECTOR orientationVec = DirectX::XMLoadFloat4(&orientation);
        DirectX::XMVECTOR addOrientation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), 90);
        orientationVec = DirectX::XMQuaternionMultiply(orientationVec, addOrientation);
        DirectX::XMStoreFloat4(&orientation, orientationVec);
        SetQuaternionRotation(orientation);
        doorState = DoorState::Open;
    }
    break;
    case DoorLeftActor::DoorState::Open:

        break;
    default:
        break;
    }
}

// プレイヤーが押した時に呼ぶ 
void DoorLeftActor::Interact()
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}

void DoorLeftActor::DrawImGuiDetails()
{
    if (ImGui::Button(U8("ドアが開く")))
    {
        Interact();
    }
}


void DoorRightActor::Initialize(const Transform& transform)
{
    std::string parentName = "Door_Right";

    // ドアのメッシュコンポーネントを追加
    meshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    meshComponent->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
    // ドアのサイズを取得
    DirectX::XMFLOAT3 size = meshComponent->GetModelSize();

#if 1
    // ドアの当たり判定用のコリジョンコンポーネントを追加
    std::shared_ptr<BoxComponent> boxComponent = AddComponent<BoxComponent>("DoorCollision", parentName);
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,-1.1f });
    boxComponent->SetCollisionOffsetY(size.y * 0.5f);
    boxComponent->SetCollisionOffsetX(-size.x * 0.5f);
    boxComponent->SetCollisionOffsetZ(-size.z * 0.5f);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldProps);
    //boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();
#endif
    //// ドアにトリガー用のコリジョンコンポーネントを追加
    //auto interactTrigger = AddComponent<BoxComponent>("DoorTrigger", parentName);
    //DirectX::XMFLOAT3 triggerSize = { size.x * 1.2f ,size.y * 1.2f , size.z * 1.2f };// 少し大きくする
    //interactTrigger->SetBoxExtent(triggerSize);
    //interactTrigger->SetCollisionOffsetY(triggerSize.y * 0.5f);
    //interactTrigger->SetCollisionOffsetX(-triggerSize.x * 0.5f);
    //interactTrigger->SetCollisionOffsetZ(-triggerSize.z * 0.5f);
    //interactTrigger->SetLayer(CollisionLayer::Interactable);
    //interactTrigger->SetResponseToLayer(
    //    CollisionLayer::Player,
    //    CollisionComponent::CollisionResponse::Trigger
    //);
    //interactTrigger->SetTrigger(true);
    //interactTrigger->Initialize();

}

void DoorRightActor::Update(float deltaTime)
{

}

// プレイヤーが押した時に呼ぶ 
void DoorRightActor::Interact()
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}
