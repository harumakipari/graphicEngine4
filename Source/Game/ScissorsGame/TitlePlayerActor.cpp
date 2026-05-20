#include "pch.h"
#include "TitlePlayerActor.h"

void TitlePlayerActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Player/TitlePlayer.gltf", false, true);

    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());

    controller->AddAnimation("Idle", 0);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    this->GetAnimationController()->SetAnimationRate(0.5f);
    PlayAnimation("Idle");
}

void TitlePlayerActor::Update(float deltaTime)
{
    Character::Update(deltaTime);

}

void LoadingPlayerActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Player/LoadingPlayer.gltf", false, true);
    
    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());

    controller->AddAnimation("Idle", 0);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);
    this->GetAnimationController()->SetAnimationRate(1.0f);
    PlayAnimation("Idle");

}

void LoadingPlayerActor::Update(float deltaTime)
{
    Character::Update(deltaTime);
}

