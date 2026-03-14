#include "pch.h"
#include "SkeletonWarriorEnemy.h"

void SkeletonWarriorActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/Models/Characters/Skeleton/Skeleton.gltf");
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定

    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    //controller->AddAnimation("Idle", 0);
    controller->AddAnimation("Attack", 0);
    //controller->AddAnimation("Block", 2);
    //controller->AddAnimation("Death", 3);
    //controller->AddAnimation("BlockIdle", 4);
    //controller->AddAnimation("Run", 5);
    //controller->AddAnimation("Walk", 6);
    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);

    PlayAnimation("Attack");

    waypoints =
    {
        { -15.0f, 0.0f, 12.0f },
        { -10.0f, 0.0f, 12.0f },
        { -10.0f, 0.0f, 18.0f },
        { -15.0f, 0.0f, 18.0f }
    };

    shield = AddComponent<SkeletalMeshComponent>("ShieldMesh", parentName);
    shield->SetModel("./Data/Models/Weapons/Shield/Shield.gltf");
    //DirectX::XMFLOAT4X4 shieldMatrix = skeletalMeshComponent->model->GetJointMatrix("Hand_l", skeletalMeshComponent->modelNodes);
    //shield->SetRelativeMatrixDirect(shieldMatrix);

    sword = AddComponent<SkeletalMeshComponent>("SwordMesh", parentName);
    sword->SetModel("./Data/Models/Weapons/Sword/Sword.gltf");
    //DirectX::XMFLOAT4X4 swordMatrix = skeletalMeshComponent->model->GetJointMatrix("Hand_r", skeletalMeshComponent->modelNodes);
    //sword->SetRelativeMatrixDirect(swordMatrix);
}

void SkeletonWarriorActor::Update(float elapsedTime)
{
    Character::Update(elapsedTime);
    DirectX::XMFLOAT4X4 matrix = GetWorldTransform();

    //DirectX::XMFLOAT4X4 swordMatrix = skeletalMeshComponent->model->GetJointLocalMatrix("Hand_r", skeletalMeshComponent->modelNodes);
    DirectX::XMFLOAT4X4 swordMatrix = skeletalMeshComponent->model->GetJointMatrix("Hand_r", skeletalMeshComponent->modelNodes, matrix);
    //DirectX::XMFLOAT3 swordPosition = skeletalMeshComponent->model->GetJointWorldPosition("Hand_r", skeletalMeshComponent->modelNodes, matrix);
    sword->SetWorldMatrixDirect(swordMatrix);
    //sword->SetRelativeMatrixDirect(swordMatrix);
    //DirectX::XMFLOAT4X4 shieldMatrix = skeletalMeshComponent->model->GetJointLocalMatrix("Hand_l", skeletalMeshComponent->modelNodes);
    DirectX::XMFLOAT4X4 shieldMatrix = skeletalMeshComponent->model->GetJointMatrix("Hand_l", skeletalMeshComponent->modelNodes, matrix);
    //shield->SetRelativeMatrixDirect(shieldMatrix);
    shield->SetWorldMatrixDirect(shieldMatrix);


    //if (waypoints.empty()) return;

    //DirectX::XMFLOAT3 position = GetPosition();

    //DirectX::XMFLOAT3 target = waypoints[currentWaypoint];

    //DirectX::XMFLOAT3 dir =
    //{
    //    target.x - position.x,
    //    target.y - position.y,
    //    target.z - position.z
    //};

    //float length = sqrt(dir.x * dir.x + dir.z * dir.z);

    //if (length < 0.3f)
    //{
    //    currentWaypoint = (currentWaypoint + 1) % waypoints.size();
    //    return;
    //}

    //dir.x /= length;
    //dir.z /= length;

    //position.x += dir.x * moveSpeed * elapsedTime;
    //position.z += dir.z * moveSpeed * elapsedTime;

    //SetPosition(position);
}