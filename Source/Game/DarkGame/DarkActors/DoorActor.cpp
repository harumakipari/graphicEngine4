#include "pch.h"
#include "DoorActor.h"

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
    float delta = openSpeed * deltaTime;

    switch (doorState)
    {

    case DoorState::Opening:

        openAngle += delta;

        if (openAngle >= 90)
        {
            delta -= (openAngle - 90);
            doorState = DoorState::Open;
        }

        leftHinge->AddLocalRotation({ 0,-delta,0 });
        rightHinge->AddLocalRotation({ 0, delta,0 });

        break;


    case DoorState::Closing:

        openAngle -= delta;

        if (openAngle <= 0)
        {
            delta -= (0 - openAngle);
            doorState = DoorState::Closed;
        }

        leftHinge->AddLocalRotation({ 0, delta,0 });
        rightHinge->AddLocalRotation({ 0,-delta,0 });

        break;

    default:
        break;
    }
}

void DoorActor::Interact()
{
    if (doorState == DoorState::Closed || doorState == DoorState::Closing)
    {
        doorState = DoorState::Opening;
    }
    else if (doorState == DoorState::Open || doorState == DoorState::Opening)
    {
        doorState = DoorState::Closing;
    }
}