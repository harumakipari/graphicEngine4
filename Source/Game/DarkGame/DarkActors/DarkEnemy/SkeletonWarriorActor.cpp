#include "pch.h"
#include "SkeletonWarriorActor.h"

void SkeletonWarriorActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/Models/Characters/SkeletonWarrior/SkeletonWarrior.gltf");
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Player;   // オブジェクトの種類を Player に設定

    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    controller->AddAnimation("Idle", 0);
    controller->AddAnimation("Attack", 1);
    controller->AddAnimation("Block", 2);
    controller->AddAnimation("Death", 3);
    controller->AddAnimation("BlockIdle", 4);
    controller->AddAnimation("Run", 5);
    controller->AddAnimation("Walk", 6);
    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);

    PlayAnimation("Walk");

    waypoints =
    {
        { -15.0f, 0.0f, 12.0f },
        { -10.0f, 0.0f, 12.0f },
        { -10.0f, 0.0f, 18.0f },
        { -15.0f, 0.0f, 18.0f }
    };

}

void SkeletonWarriorActor::Update(float elapsedTime)
{
    Character::Update(elapsedTime);
    if (waypoints.empty()) return;

    DirectX::XMFLOAT3 position = GetPosition();

    DirectX::XMFLOAT3 target = waypoints[currentWaypoint];

    DirectX::XMFLOAT3 dir =
    {
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    };

    float length = sqrt(dir.x * dir.x + dir.z * dir.z);

    if (length < 0.3f)
    {
        currentWaypoint = (currentWaypoint + 1) % waypoints.size();
        return;
    }

    dir.x /= length;
    dir.z /= length;

    position.x += dir.x * moveSpeed * elapsedTime;
    position.z += dir.z * moveSpeed * elapsedTime;

    SetPosition(position);
}