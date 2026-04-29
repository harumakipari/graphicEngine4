#include "pch.h"
#include "YarnWallActor.h"

#include "Game/Actors/Base/Character.h"


void YarnWallActor::Initialize(const Transform& transform)
{
    std::string parentName = "YarnWallActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetRelativeLocationDirect({ 0, 0.0f, 0 });
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/YarnWallModel.glb");


    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    // ”½ŽË—p‚Ì“–‚½‚è”»’è
    redirectCollisionComponent = this->AddComponent<BoxComponent>("redirectCollisionComponent", parentName);
    redirectCollisionComponent->SetBoxExtent(size);
    redirectCollisionComponent->SetMass(0.0f);
    redirectCollisionComponent->SetLayer(CollisionLayer::EnemyRedirect);
    redirectCollisionComponent->Initialize();
}

void YarnWallActor::Update(float deltaTime)
{
    
}

