#include "pch.h"
#include "OdenTitleStageActor.h"

void OdenTitleStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "RootComponent";

    storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Yatai.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    storeModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    auto groundModelComponent=AddComponent<StaticMeshComponent>("Oden_Ground_Model", parentName);
    groundModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Ground.gltf", false);
    groundModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    groundModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
}


