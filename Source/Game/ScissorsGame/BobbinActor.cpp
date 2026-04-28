#include "pch.h"
#include "BobbinActor.h"

void BobbinActor::Initialize(const Transform& transform)
{
    std::string parentName = "BobbinActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/BobbinModel.glb", false, true);
}

void BobbinActor::Update(float deltaTime)
{
    
}
