#include "pch.h"
#include "OdenStoreActor.h"

void OdenStoreActor::Initialize(const Transform& transform)
{
    auto staticMeshComponent=AddComponent<StaticMeshComponent>("Oden_Store");
    staticMeshComponent->SetModel("./Data/Models/Oden_Store/Oden_frame.gltf", false);
    staticMeshComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
}
