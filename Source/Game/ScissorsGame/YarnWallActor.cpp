#include "pch.h"
#include "YarnWallActor.h"

#include "Game/Actors/Base/Character.h"


void YarnWallActor::Initialize(const Transform& transform)
{
    std::string parentName = "YarnWallActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetRelativeLocationDirect({ 0, 0.0f, 0 });
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/YarnWallModel.glb");

    // ”½ŽË—p‚Ì“–‚½‚è”»’è
    redirectCollisionComponent = this->AddComponent<BoxComponent>("redirectCollisionComponent", parentName);
    redirectCollisionComponent->SetBoxExtent({ 3.0f,1.0f,1.0f });
    redirectCollisionComponent->SetMass(0.0f);
    redirectCollisionComponent->SetLayer(CollisionLayer::EnemyRedirect);
    redirectCollisionComponent->Initialize();
}

void YarnWallActor::Update(float deltaTime)
{
    
}

