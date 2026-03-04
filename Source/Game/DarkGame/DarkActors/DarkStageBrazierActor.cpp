#include "pch.h"
#include "DarkStageBrazierActor.h"

#include "Components/Render/PointLightComponent.h"
#include "Engine/Scene/SceneBase.h"

void DarkStageBrazierActor::Initialize(const Transform& transform)
{
    std::string parentName = "candelabraMesh";

    // 火鉢のモデルを追加
    brazierMeshComponent = this->AddComponent<SkeletalMeshComponent>(parentName);
    brazierMeshComponent->SetModel("./Data/Models/DarkStageAssets/Brazier/Brazier.gltf");
    brazierMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする

    auto scene = dynamic_cast<SceneBase*>(Scene::GetCurrentScene());
    auto lightManager = scene->GetLightManager();


    auto lightsData = brazierMeshComponent->model->GetPointLights();
    // ポイントライトコンポーネントを追加
    for (int i = 0; i < static_cast<int>(lightsData.size()); ++i)
    {
        const auto& light = lightsData[i];

        std::string compName = "pointLightComponent_" + light.name + std::to_string(i);
        auto pointLightComponent =
            this->AddComponent<PointLightComponent>(compName, parentName);

        DirectX::XMFLOAT3 pos = MathHelper::ConvertRHtoLh(light.worldPosition);
        pointLightComponent->SetRelativeLocationDirect(pos);
        // ライトの名前からライトマネージャーの共有ライトを取得して設定
        pointLightComponent->SetSharedParam(&lightManager->GetSharedLight(light.name));


    }

    for (auto point : brazierMeshComponent->model->spawnPoints)
    {
        // エミッションを発生させるためにモデルを追加
        auto sphereMeshComponent = this->AddComponent<SkeletalMeshComponent>("sphereMeshComponent", parentName);
        sphereMeshComponent->SetModel("./Data/Models/Primitives/Sphere.glb");
        sphereMeshComponent->overrideDeferredPipelineName = "pointLightSkeletalMesh";
        sphereMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする
        sphereMeshComponent->SetRelativeScaleDirect({ 0.01f,0.01f,0.01f });
        DirectX::XMFLOAT3 pos = MathHelper::ConvertRHtoLh(point.worldPosition);
        pos.y += 0.1f;
        sphereMeshComponent->SetRelativeLocationDirect(pos);
        sphereMeshComponent->SetRelativeRotationDirect(point.worldRotation);
        sphereMeshComponent->cpuColor = { 1,0.2f,0,1 };
        sphereMeshComponent->emissionPower = 6.0f;
    }

}
