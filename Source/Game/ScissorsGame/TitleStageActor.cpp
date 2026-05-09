#include "pch.h"
#include "TitleStageActor.h"

void TitleStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "TrailModelActor";
    stageModelComponent = AddComponent<SkeletalMeshComponent>(parentName);
    stageModelComponent->SetModel("./Data/TeamModels/Title/TitleStageModel.gltf", false, true);
}

void TitleStageActor::Update(float deltaTime)
{
}

