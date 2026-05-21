#include "pch.h"
#include "TitleStageActor.h"

void TitleStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "TrailModelActor";
    stageModelComponent = AddComponent<SkeletalMeshComponent>(parentName);
    stageModelComponent->SetModel("./Data/TeamModels/Title/TitleStageModel.gltf", false, true);

    titleLogoComponent = AddComponent<SkeletalMeshComponent>("titleLogo",parentName);
    titleLogoComponent->SetModel("./Data/TeamModels/Title/title_rogo.gltf", false, true);
    titleLogoComponent->SetIsCastShadow(false);
    titleLogoComponent->SetRelativeLocationDirect({ 0.0f,-0.45f,0.0f });

}

void TitleStageActor::Update(float deltaTime)
{
}

