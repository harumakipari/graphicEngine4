#include "pch.h"
#include "TrailModelActor.h"

void TrailModelActor::Initialize(const Transform& transform)
{
    std::string parentName = "TrailModelActor";

    trailModelMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    trailModelMeshComponent->SetModel("./Data/TeamModels/Player/TrailModel.gltf", false, true);
    trailModelMeshComponent->SetIsCastShadow(false);
}

void TrailModelActor::Update(float deltaTime)
{
  
}

void TrailModelActor::SetDirection(DirectX::XMFLOAT3 dir)
{
    float yaw = atan2f(dir.x, dir.z);
    yaw = DirectX::XMConvertToDegrees(yaw);
    trailModelMeshComponent->SetWorldEulerRotationDirect(
        {
            0.0f,
            yaw,
            0.0f
        });
}