#include "pch.h"
#include "DarkStagePointLightActor.h"

void DarkStagePointLightActor::Initialize(const Transform& transform)
{
    std::string parentName = "pointLight";
    // ポイントライトコンポーネントを追加
    pointLightComponent = this->AddComponent<PointLightComponent>(parentName);

    // エミッションを発生させるためにモデルを追加
    sphereMeshComponent = this->AddComponent<SkeletalMeshComponent>("sphereMeshComponent", parentName);
    sphereMeshComponent->SetModel("./Data/Models/Primitives/Sphere.glb");
    sphereMeshComponent->overrideDeferredPipelineName = "pointLightSkeletalMesh";
    sphereMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする
    sphereMeshComponent->SetRelativeScaleDirect({ 0.01f,0.01f,0.01f });
}

// ポイントライトのデータを一括で設定する関数
void DarkStagePointLightActor::SetPointLightData(const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 color, const float intensity, const float range)
{
    if (pointLightComponent)
    {
        pointLightComponent->SetColor(color);
        pointLightComponent->SetIntensity(intensity);
        pointLightComponent->SetRange(range);
    }
}

