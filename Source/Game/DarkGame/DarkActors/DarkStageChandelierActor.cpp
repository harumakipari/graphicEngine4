#include "pch.h"
#include "DarkStageChandelierActor.h"

void DarkStageChandelierActor::Initialize(const Transform& transform)
{
    std::string parentName = "chandelierMesh";

    // シャンデリアのモデルを追加
    chandelierMeshComponent = this->AddComponent<SkeletalMeshComponent>( parentName);
    chandelierMeshComponent->SetModel("./Data/Models/DarkStageAssets/Chandelier/Chandelier.gltf");
    chandelierMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする

    auto lightsData = chandelierMeshComponent->model->GetPointLights();
    // ポイントライトコンポーネントを追加
    for (int i = 0; i < static_cast<int>(lightsData.size()); ++i)
    {
        const auto& light = lightsData[i];
#if 1
        std::string compName = "pointLightComponent_" + std::to_string(i);
        auto pointLightComponent =
            this->AddComponent<PointLightComponent>(compName, parentName);

        DirectX::XMFLOAT3 pos = MathHelper::ConvertRHtoLh(light.worldPosition);
        pointLightComponent->SetRelativeLocationDirect(pos);
        pointLightComponent->SetColor(light.color);
        pointLightComponent->SetRange(light.range);
        pointLightComponent->SetIntensity(light.intensity);
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
}

void DarkStageChandelierActor::Update(float deltaTime)
{
    swingTime += deltaTime;

    float angle = sinf(swingTime * swingSpeed) * swingAngle;
    DirectX::XMFLOAT3 rot = { angle, 0.0f, 0.0f }; // X軸回転で揺らす
    chandelierMeshComponent->SetRelativeEulerRotationDirect(rot);
}


