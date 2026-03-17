#include "pch.h"
#include "SkeletonWarriorEnemy.h"

void SkeletonWarriorActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    //skeletalMeshComponent->SetModel("./Data/Models/Characters/Skeleton/Skeleton.gltf");
    skeletalMeshComponent->SetModel("./Data/Models/Characters/Skeleton/SK_Skeleton_01.gltf");
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP; // 手にちゃんとつけるために

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

    // 盾
    shield = AddComponent<SkeletalMeshComponent>("ShieldMesh", parentName);
    shield->SetModel("./Data/Models/Weapons/Shield/Shield.gltf");
    shield->AttachToComponent(skeletalMeshComponent, 11); // "Hand_l_end"

    // 剣
    sword = AddComponent<SkeletalMeshComponent>("SwordMesh", parentName);
    sword->SetModel("./Data/Models/Weapons/Sword/Sword.gltf");
    sword->AttachToComponent(skeletalMeshComponent, 16); // "Hand_r_end"
    sword->SetRelativeLocationDirect({ -0.f, -0.1f, -0.0f });
    sword->SetRelativeEulerRotationDirect({ 0.0f, 90.f, 0.0f });
    sword->SetRelativeScaleDirect({ 0.8f,0.8f,0.8f });

    // 当たり判定
    {
        std::shared_ptr<CapsuleComponent> capsuleComponent = this->AddComponent<class CapsuleComponent>("capsuleComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        height = size.y;
        radius = size.x * 0.5f;
        mass = 60.0f;
        capsuleComponent->SetRadiusAndHeight(radius, height);
        capsuleComponent->SetMass(mass);
        capsuleComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        capsuleComponent->SetLayer(CollisionLayer::Enemy);
        capsuleComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::WorldProps, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
        capsuleComponent->SetCollisionOffsetY(height * 0.5f);
        capsuleComponent->SetIsVisibleDebugBox(false);
        capsuleComponent->Initialize();
    }

}

void SkeletonWarriorActor::Update(float elapsedTime)
{
    Character::Update(elapsedTime);

    //if (waypoints.empty()) return;

    //DirectX::XMFLOAT3 position = GetPosition();

    //position.z += 0.1f * elapsedTime;
    //SetPosition(position);

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