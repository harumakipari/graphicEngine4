#include "pch.h"
#include "TitleBookActor.h"

void TitleBookActor::Initialize(const Transform& transform)
{
    std::string parentName = "TitleBookActor";
    auto rootComponent= AddComponent<SceneComponent>(parentName);

    bookRightModel = AddComponent<SkeletalMeshComponent>("bookRightModel", parentName);
    bookRightModel->SetModel("./Data/TeamModels/Title/BookRight.gltf", false, false);


    bookLeftModel = AddComponent<SkeletalMeshComponent>("bookLeftModel",parentName);
    bookLeftModel->SetModel("./Data/TeamModels/Title/BookLeft.gltf", false, false);
    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,180.0f });

    
}

void TitleBookActor::Update(float deltaTime)
{
    
}
