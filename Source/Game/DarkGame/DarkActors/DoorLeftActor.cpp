#include "pch.h"
#include "DoorLeftActor.h"

void DoorLeftActor::Initialize(const Transform& transform)
{
    std::string parentName = "Door_Left";

    // ドアのメッシュコンポーネントを追加
    meshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    meshComponent->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
    // ドアのサイズを取得
    DirectX::XMFLOAT3 size = meshComponent->GetModelSize();

    // ドアの当たり判定用のコリジョンコンポーネントを追加
    std::shared_ptr<BoxComponent> boxComponent = AddComponent<BoxComponent>("DoorCollision", parentName);
    boxComponent->SetBoxExtent(size);
    boxComponent->SetCollisionOffsetY(size.y * 0.5f);
    boxComponent->SetCollisionOffsetX(-size.x * 0.5f);
    boxComponent->SetCollisionOffsetZ(-size.z * 0.5f);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldStatic);
    boxComponent->SetResponseToLayer(CollisionLayer::Player,CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

}

void DoorLeftActor::Update(float deltaTime)
{

}

// プレイヤーが押した時に呼ぶ 
void DoorLeftActor::Interact()
{
    if (doorState == DoorState::Closed)
        doorState = DoorState::Opening;
}
