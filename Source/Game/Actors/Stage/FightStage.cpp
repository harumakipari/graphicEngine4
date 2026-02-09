#include "pch.h"
#include  "FightStage.h"

#include "Components/Effect/ParticleComponent.h"

void FightStage::Initialize(const Transform& transform)
{
    std::string parentName = "staticMeshComponent";

    std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->AddComponent<class StaticMeshComponent>(parentName);
#if 0
    staticMeshComponent->SetModel("./Data/Models/Stage/ExampleStage.gltf", true);
#else
    staticMeshComponent->SetModel("./Data/Models/Dark_Stage0209/DarkStage.gltf", true);
    staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
    auto lightsData = staticMeshComponent->model->GetPointLights();
    // ポイントライトコンポーネントを追加
    for (int i = 0; i < static_cast<int>(lightsData.size()); ++i)
    {
        const auto& light = lightsData[i];

        std::string compName = "pointLightComponent_" + std::to_string(i);

        auto pointLightComponent =
            this->AddComponent<PointLightComponent>(compName, parentName);

        pointLightComponent->SetRelativeLocationDirect(light.position);
        pointLightComponent->SetColor(light.color);
        pointLightComponent->SetRange(light.range);
        pointLightComponent->SetIntensity(light.intensity);
    }


    //staticMeshComponent->SetModel("./Data/Models/Dark_Stage0204/untitled.gltf", true);
    //staticMeshComponent->SetModel("./Data/Models/Dark_Stage/Mesh/untitled.gltf", true);
    //staticMeshComponent->SetModel("./Data/Models/boss_fight_stage/scene.gltf", true);
    //staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
    //staticMeshComponent->overrideDeferredPipelineName = "deferredFightStage";
    //staticMeshComponent->hueShift = 191.8f;
    //staticMeshComponent->saturation = 0.8f;
    //staticMeshComponent->brightness = 1.0f;
    //HRESULT hr = CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelFightStagePS.cso", staticMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif // 1

#if 1// 当たり判定
    // メッシュ
    std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent", parentName);
    triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
    triangleMeshComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    triangleMeshComponent->CreateConvexMeshFromModel(staticMeshComponent.get());
#else

    std::shared_ptr<BoxComponent> boxComponent = this->AddComponent<class BoxComponent>("boxComponent", "staticMeshComponent");
    boxComponent->SetHalfBoxExtent(DirectX::XMFLOAT3(40.0f, 0.2f, 40.0f));
    //boxComponent->SetCollisionOffsetY(-4.5f);
    boxComponent->SetStatic(true);
    boxComponent->SetLayer(CollisionLayer::WorldStatic);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();

#endif // 0 // 当たり判定

#if 0
    // 湯気のエフェクト
    auto steamComponent = this->AddComponent<ParticleComponent>("steamComponent", parentName);
    DirectX::XMFLOAT3 potPosition = staticMeshComponent->model->GetJointLocalPosition("SM_FirePot_01", staticMeshComponent->model->GetNodes());
    steamComponent->Load("./Data/Effect/Files/Steam_Pot_Effect.json");
    steamComponent->SetRelativeLocationDirect(potPosition);
    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
        //.startDelay = 0.5f // 再生開始遅延時間
    };
    steamComponent->SetAddSettings(settings);
    steamComponent->Play();

#endif // 0
}
