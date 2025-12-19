#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

class FightStage :public Actor
{
public:
    FightStage(std::string modelName) :Actor(modelName) {}

    void Initialize(const Transform& transform)override
    {
        std::shared_ptr<StaticMeshComponent> staticMeshComponent = this->NewSceneComponent<class StaticMeshComponent>("staticMeshComponent");
        staticMeshComponent->SetModel("./Data/Models/Stage/ExampleStage.gltf", true);
        //staticMeshComponent->SetModel("./Data/Models/boss_fight_stage/scene.gltf", true);
        staticMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        //staticMeshComponent->SetRelativeLocationDirect({ 0.0f,2.45f,0.0f });

        std::shared_ptr<TriangleMeshCollisionComponent> triangleMeshComponent = this->NewSceneComponent<class TriangleMeshCollisionComponent>("triangleMeshComponent", "staticMeshComponent");
        triangleMeshComponent->SetLayer(CollisionLayer::WorldStatic);
        triangleMeshComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        triangleMeshComponent->CreateConvexMeshFromModel(staticMeshComponent.get());
    }

    void Update(float elapsedTime)override {}
};