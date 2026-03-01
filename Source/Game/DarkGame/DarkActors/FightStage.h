#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Render/PointLightComponent.h"

class ParticleComponent;

class FightStage :public Actor
{
public:
    struct WallData
    {
        DirectX::XMFLOAT3 halfExtent;
        DirectX::XMFLOAT3 position;
    };

    std::vector<WallData> levelWalls =
    {
        { {5,5,0.5f}, {0,5,10} },
        { {0.5f,5,5}, {10,5,0} },
        { {3,3,3}, {5,3,5} },
    };
public:
    explicit FightStage(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

private:
    void BuildStage();

    std::shared_ptr<BoxComponent> CreateWall(const std::string& name, const DirectX::XMFLOAT3& halfExtent, const DirectX::XMFLOAT3& position);



private:

};
