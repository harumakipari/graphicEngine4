#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

class FightStage :public Actor
{
public:
    FightStage(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override
    {
        std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->AddComponent<class StaticMeshComponent>("staticMeshComponent");
#if 0
        staticMeshComponent->SetModel("./Data/Models/Stage/ExampleStage.gltf", true);
#else
        staticMeshComponent->SetModel("./Data/Models/Dark_Stage/Mesh/untitled.gltf", true);
        //staticMeshComponent->SetModel("./Data/Models/boss_fight_stage/scene.gltf", true);
        staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        //staticMeshComponent->overrideDeferredPipelineName = "deferredFightStage";
        //staticMeshComponent->hueShift = 191.8f;
        //staticMeshComponent->saturation = 0.8f;
        //staticMeshComponent->brightness = 1.0f;
        //HRESULT hr = CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelFightStagePS.cso", staticMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif // 1
        //staticMeshComponent->SetRelativeLocationDirect({ 0.0f,2.45f,0.0f });

#if 0 // ìñÇΩÇËîªíË
        // ÉÅÉbÉVÉÖ
        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->AddComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent", "staticMeshComponent");
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

#endif // 0 // ìñÇΩÇËîªíË

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
    }

    void Update(float elapsedTime)override {}
};