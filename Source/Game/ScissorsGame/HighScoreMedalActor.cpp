#include "pch.h"
#include "HighScoreMedalActor.h"


void HighScoreMedalActor::Initialize(const Transform& transform)
{
    std::string parentName = "HighScoreMedalActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Title/HighScoreMedalModel.gltf", false, true);
    skeletalMeshComponent->SetIsCastShadow(false);
}

void HighScoreMedalActor::Update(float elapsedTime)
{
    
}

void HighScoreMedalActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
#endif
}

// èIóπéûÇÃèàóù
void HighScoreMedalActor::Finalize()
{
    
}


void HighScoreMedalActor::Play()
{
    
}
