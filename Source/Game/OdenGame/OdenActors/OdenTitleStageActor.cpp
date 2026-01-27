#include "pch.h"
#include "OdenTitleStageActor.h"

void OdenTitleStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "RootComponent";

    storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Yatai.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    storeModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    auto groundModelComponent = AddComponent<StaticMeshComponent>("Oden_Ground_Model", parentName);
    groundModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Ground.gltf", false);
    groundModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    groundModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    // “ïˆÕ“x‘I‘ð‚Ìƒ‚ƒfƒ‹
    selectModelComponent = AddComponent<SkeletalMeshComponent>("Oden_Select_Model", "Oden_Store_Model");
    selectModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Select_Easy.gltf");
    selectModelComponent->SetRelativeLocationDirect({ 10.7f,12.4f,7.6f });
}


