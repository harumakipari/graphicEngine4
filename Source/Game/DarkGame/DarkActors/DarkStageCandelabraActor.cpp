#include "pch.h"
#include "DarkStageCandelabraActor.h"

#include "Engine/Scene/SceneBase.h"

void DarkStageCandelabraActor::Initialize(const Transform& transform)
{

}
void DarkStageCandelabraActor::SetModel(const std::shared_ptr<StageAsset>& stageAsset)
{
    std::string parentName = "candelabraMesh";

    // 燭台のモデルを追加
    candelabraMeshComponent = this->AddComponent<SkeletalMeshComponent>(parentName);
    candelabraMeshComponent->model = stageAsset->model;
    candelabraMeshComponent->SetIsCastShadow(false);    // 影を落とさないようにする

#if 0
    // サイズを取得
    DirectX::XMFLOAT3 size = stageAsset->model->GetModelSize();
    std::shared_ptr<BoxComponent> boxComponent = AddComponent<BoxComponent>("collision", parentName);
    boxComponent->SetBoxExtent(size);
    boxComponent->SetCollisionOffsetY(size.y * 0.5f);
    boxComponent->SetCollisionOffsetX(-size.x * 0.5f);
    boxComponent->SetCollisionOffsetZ(-size.z * 0.5f);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::Interactable);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();
#endif // 0


    auto scene = dynamic_cast<SceneBase*>(Scene::GetCurrentScene());
    auto lightManager = scene->GetLightManager();



    auto lightsData = candelabraMeshComponent->model->GetPointLights();
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
        // ライトの名前からライトマネージャーの共有ライトを取得して設定
        if (auto shared = lightManager->FindSharedLight(light.name))
        {
            pointLightComponent->SetSharedParam(shared);
        }

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

    for (auto point : stageAsset->spawnPoints)
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