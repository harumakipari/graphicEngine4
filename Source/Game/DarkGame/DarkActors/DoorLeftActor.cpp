#include "pch.h"
#include "DoorLeftActor.h"

void DoorLeftActor::Initialize(const Transform& transform)
{
    const std::string parentName = "Door_Left";

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
        float deltaAngle = -openSpeed * deltaTime; // ←逆方向
        openAngle += deltaAngle;

        if (openAngle <= -90.0f)
        {
            deltaAngle -= (openAngle + 90.0f); // 行き過ぎ防止
            doorState = DoorState::Open;
        }

        DirectX::XMFLOAT4 orientation = GetQuaternionRotation();
        DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&orientation);

        DirectX::XMVECTOR add =
            DirectX::XMQuaternionRotationAxis(
                DirectX::XMVectorSet(0, 1, 0, 0),
                DirectX::XMConvertToRadians(deltaAngle)
            );

        q = DirectX::XMQuaternionMultiply(q, add);

        DirectX::XMStoreFloat4(&orientation, q);

        SetQuaternionRotation(orientation);
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
    switch (doorState)
    {
    case DoorRightActor::DoorState::Closed:

        break;
    case DoorRightActor::DoorState::Opening:
    {
        float deltaAngle = openSpeed * deltaTime;
        openAngle += deltaAngle;

        if (openAngle >= 90.0f)
        {
            deltaAngle -= (openAngle - 90.0f); // 行き過ぎ防止
            doorState = DoorState::Open;
        }

        DirectX::XMFLOAT4 orientation = GetQuaternionRotation();

        DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&orientation);

        DirectX::XMVECTOR add =
            DirectX::XMQuaternionRotationAxis(
                DirectX::XMVectorSet(0, 1, 0, 0),
                DirectX::XMConvertToRadians(deltaAngle)
            );

        q = DirectX::XMQuaternionMultiply(q, add);

        DirectX::XMStoreFloat4(&orientation, q);

        SetQuaternionRotation(orientation);
    }
    break;
    case DoorRightActor::DoorState::Open:

        break;
    default:
        break;
    }
}

// プレイヤーが押した時に呼ぶ 
void DoorRightActor::Interact()
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}

void DoorActor::Initialize(const Transform& transform)
{
    root = AddComponent<SceneComponent>("DoorRoot");

    leftHinge = AddComponent<SceneComponent>("LeftHinge", "DoorRoot");
    rightHinge = AddComponent<SceneComponent>("RightHinge", "DoorRoot");

    leftDoorMesh = AddComponent<SkeletalMeshComponent>("LeftDoor", "LeftHinge");
    rightDoorMesh = AddComponent<SkeletalMeshComponent>("RightDoor", "RightHinge");

    // ドアのメッシュコンポーネントを追加
    leftDoorMesh->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
    rightDoorMesh->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");

    // ドアのサイズを取得
    DirectX::XMFLOAT3 leftSize = leftDoorMesh->GetModelSize();
    // ドアの当たり判定用のコリジョンコンポーネントを追加
    std::shared_ptr<BoxComponent> leftBoxComponent = AddComponent<BoxComponent>("DoorLeftCollision", "LeftHinge");
    leftBoxComponent->SetBoxExtent(leftSize);
    leftBoxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,-1.1f });
    leftBoxComponent->SetCollisionOffsetY(leftSize.y * 0.5f);
    leftBoxComponent->SetCollisionOffsetX(-leftSize.x * 0.5f);
    leftBoxComponent->SetCollisionOffsetZ(-leftSize.z * 0.5f);
    leftBoxComponent->SetStatic(true);
    leftBoxComponent->SetLayer(CollisionLayer::WorldProps);
    leftBoxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    leftBoxComponent->Initialize();

    // ドアのサイズを取得
    DirectX::XMFLOAT3 rightSize = rightDoorMesh->GetModelSize();
    // ドアの当たり判定用のコリジョンコンポーネントを追加
    std::shared_ptr<BoxComponent> rightBoxComponent = AddComponent<BoxComponent>("DoorRightCollision", "RightHinge");
    rightBoxComponent->SetBoxExtent(rightSize);
    rightBoxComponent->SetBoxExtent(rightSize);
    rightBoxComponent->SetRelativeLocationDirect({ 0.0f,0.0f,-1.1f });
    rightBoxComponent->SetCollisionOffsetY(rightSize.y * 0.5f);
    rightBoxComponent->SetCollisionOffsetX(-rightSize.x * 0.5f);
    rightBoxComponent->SetCollisionOffsetZ(-rightSize.z * 0.5f);
    rightBoxComponent->SetStatic(true);
    rightBoxComponent->SetLayer(CollisionLayer::WorldProps);
    rightBoxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    rightBoxComponent->Initialize();

    // ヒンジ位置調整
    leftHinge->SetRelativeLocationDirect({ 0.0f,0,-2.0f });
    leftHinge->SetRelativeEulerRotationDirect({ 0.0f,180.0f,0.0f });
    rightHinge->SetRelativeLocationDirect({ 0.0f,0,2.0f });
}

void DoorActor::Update(float deltaTime)
{
    if (doorState != DoorState::Opening)
        return;

    float delta = openSpeed * deltaTime;
    openAngle += delta;

    if (openAngle >= 90)
    {
        delta -= (openAngle - 90);
        doorState = DoorState::Open;
    }

    leftHinge->AddLocalRotation({ 0,-delta,0 });
    rightHinge->AddLocalRotation({ 0,delta,0 });
}

void DoorActor::Interact()
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}