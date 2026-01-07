#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

class WaterSphere :public Actor
{
public:
    WaterSphere(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override
    {
        std::shared_ptr<SkeletalMeshComponent> skeletalMesh = AddComponent<SkeletalMeshComponent>("skeletalMesh");
        skeletalMesh->SetModel("./Data/Models/Morph/sphere.glb", true);
    }

    void Update(float elapsedTime)override {}
};