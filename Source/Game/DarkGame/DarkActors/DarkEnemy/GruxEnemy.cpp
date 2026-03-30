#include "pch.h"

#include "GruxEnemy.h"

#include "Components/Render/PointLightComponent.h"
#include "Engine/Scene/SceneBase.h"

void GruxEnemy::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/Models/Characters/Grux/animations.gltf", false, true);
    //skeletalMeshComponent->SetModel("./Data/Models/Characters/StoneGolem/StoneGolem.gltf", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // 目玉の自己発光の強さを設定
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 0.9f,0.08f,0.08f,1.0f };   // 目玉の色を赤にしてみる
    for (auto& material : skeletalMeshComponent->model->materials)
    {
        if (material.name == "MI_Grux_Eye")
        {// 目だったら、
            material.materialType = MaterialType::Eye;
        }
    }


    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    controller->AddAnimation("Idle", 0);
    controller->AddAnimation("PrimaryAttack_RA", 1);
    controller->AddAnimation("PrimaryAttack_LA", 2);
    controller->AddAnimation("PrimaryAttack_LB", 3);
    controller->AddAnimation("PrimaryAttack_RB", 4);
    controller->AddAnimation("Respawn", 5);
    controller->AddAnimation("LaunchPad", 6);
    controller->AddAnimation("LevelStart", 7);
    controller->AddAnimation("Death", 8);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    PlayAnimation("Idle");

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

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);


}

void GruxEnemy::Update(float elapsedTime)
{
    Character::Update(elapsedTime);

    DirectX::XMFLOAT3 position = GetPosition();

    SetPosition(position);
}