#include "pch.h"
#include "DarkStageChandelierActor.h"

#include "Engine/Scene/SceneBase.h"

void DarkStageChandelierActor::Initialize(const Transform& transform)
{
    std::string parentName = "chandelierMesh";

    // シャンデリアのモデルを追加
    chandelierMeshComponent = this->AddComponent<SkeletalMeshComponent>(parentName);
    chandelierMeshComponent->SetModel("./Data/Models/DarkStageAssets/Chandelier/Chandelier.gltf");
    chandelierMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする

    auto scene = dynamic_cast<SceneBase*>(Scene::GetCurrentScene());

    auto lightManager = scene->GetLightManager();



    auto lightsData = chandelierMeshComponent->model->GetPointLights();
    // ポイントライトコンポーネントを追加
    for (int i = 0; i < static_cast<int>(lightsData.size()); ++i)
    {
        const auto& light = lightsData[i];

        
#if 1
        std::string compName = "pointLightComponent_" + std::to_string(i);
        auto pointLightComponent =
            this->AddComponent<PointLightComponent>(compName, parentName);

        // ライトの名前からライトマネージャーの共有ライトを取得して設定
        if (auto shared = lightManager->FindSharedLight(light.name))
        {
            pointLightComponent->SetSharedParam(shared);
        }


        DirectX::XMFLOAT3 pos = MathHelper::ConvertRHtoLh(light.worldPosition);
        pointLightComponent->SetRelativeLocationDirect(pos);
        //pointLightComponent->SetColor(light.color);
        //pointLightComponent->SetRange(light.range);
        //pointLightComponent->SetIntensity(light.intensity);
#else
        DirectX::XMFLOAT3 pos = convertRHtoLh(light.worldPosition);

        Transform pointLightTr{
            pos,
            light.worldRotation,
            light.worldScale
        };
        auto pointLightActor = scene->GetActorManager()->CreateAndRegisterActorWithTransform<DarkStagePointLightActor>("pointLight", pointLightTr);
        pointLightActor->SetPointLightData(pos, light.color, light.intensity, light.range);
#endif // 0
    }

    for (auto point : chandelierMeshComponent->model->spawnPoints)
    {
        // エミッションを発生させるためにモデルを追加
        auto sphereMeshComponent = this->AddComponent<SkeletalMeshComponent>("sphereMeshComponent", parentName);
        sphereMeshComponent->SetModel("./Data/Models/Primitives/Sphere.glb");
        sphereMeshComponent->overrideDeferredPipelineName = "pointLightSkeletalMesh";
        sphereMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする
        sphereMeshComponent->SetRelativeScaleDirect({ 0.01f,0.01f,0.01f });
        DirectX::XMFLOAT3 pos = MathHelper::ConvertRHtoLh(point.worldPosition);
        pos.y = -1.5f; // ろうそくの位置に合わせて微調整
        sphereMeshComponent->SetRelativeLocationDirect(pos);
        sphereMeshComponent->SetRelativeRotationDirect(point.worldRotation);
        sphereMeshComponent->cpuColor = { 1,0.2f,0,1 };
        sphereMeshComponent->emissionPower = 6.0f;
    }

}

void DarkStageChandelierActor::Update(float deltaTime)
{
    swingTime += deltaTime;

    float angle = sinf(swingTime * swingSpeed) * swingAngle;
    DirectX::XMFLOAT3 rot = { angle, 0.0f, 0.0f }; // X軸回転で揺らす
    chandelierMeshComponent->SetRelativeEulerRotationDirect(rot);
}


