#include "pch.h"
#include  "FightStage.h"

#include "DoorLeftActor.h"
#include "Components/Effect/ParticleComponent.h"
#include "Engine/Scene/Scene.h"

void FightStage::Initialize(const Transform& transform)
{
    std::string parentName = "staticMeshComponent";

    auto scene = GetOwnerScene();

    std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->AddComponent<class StaticMeshComponent>(parentName);
    staticMeshComponent->SetModel("./Data/Models/DarkStage0223_3/DarkStage.gltf", true);
    staticMeshComponent->SetIsCastShadow(false);
    //staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
    auto lightsData = staticMeshComponent->model->GetPointLights();
    // ポイントライトコンポーネントを追加
    for (int i = 0; i < static_cast<int>(lightsData.size()); ++i)
    {
        continue; // とりあえずポイントライトは無効化
        const auto& light = lightsData[i];

        std::string compName = "pointLightComponent_" + std::to_string(i);

        auto pointLightComponent =
            this->AddComponent<PointLightComponent>(compName, parentName);

        pointLightComponent->SetRelativeLocationDirect(light.position);
        pointLightComponent->SetColor(light.color);
        pointLightComponent->SetRange(light.range);
        pointLightComponent->SetIntensity(light.intensity);
    }


    for (auto point : staticMeshComponent->model->spawnPoints)
    {
        if (point.name == "Spawn_Door_Left")
        {
            Transform doorLeftTr{ XMFLOAT3{point.worldPosition},point.worldRotation,point.worldScale };
            auto stage = scene->GetActorManager()->CreateAndRegisterActorWithTransform<DoorLeftActor>("door_Left", doorLeftTr);

#if 0
            std::shared_ptr<SkeletalMeshComponent> door = AddComponent<SkeletalMeshComponent>("Left_Door", parentName);
            door->SetModel("./Data/Models/DarkStageAssets/Door_Large/SM_Door_Large_01.gltf");
            door->SetRelativeLocationDirect(point.worldPosition);
            door->SetRelativeRotationDirect(point.worldRotation);
            door->SetRelativeScaleDirect(point.worldScale);
#endif // 0
        }
        if (point.name == "Spawn_Particle_Steam")
        {
            // 湯気のエフェクト
            steamComponent = this->AddComponent<ParticleComponent>("steamComponent", parentName);
            steamComponent->Load("./Data/Effect/Files/Pot_SteamEffect.json");
            steamComponent->SetRelativeLocationDirect(point.worldPosition);
            // ループ再生設定
            ParticleComponent::AddSettings settings
            {
                .loop = true, // ループ再生
                //.startDelay = 0.5f // 再生開始遅延時間
            };
            steamComponent->SetAddSettings(settings);
            steamComponent->Play();
        }
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


    // 影用のスタティックメッシュコンポーネントを追加
    std::shared_ptr<StaticMeshComponent> castStaticMeshComponent = this->AddComponent<class StaticMeshComponent>("castShadowModel",parentName);
    castStaticMeshComponent->SetModel("./Data/Models/DarkStageShadow/DarkStageShadow.gltf");
    castStaticMeshComponent->SetIsVisible(false);

#if 0
    // 当たり判定
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

void FightStage::Update(float elapsedTime)
{
    //if (steamComponent)
    //{
    //    if (!steamComponent->IsPlaying())
    //    {
    //        steamComponent->Play();
    //    }
    //}

}
