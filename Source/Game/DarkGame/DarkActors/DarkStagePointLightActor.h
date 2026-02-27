#pragma once
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStagePointLightActor :public Actor
{
public:
    DarkStagePointLightActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStagePointLightActor() = default;
    virtual void Initialize(const Transform& transform)override
    {
        pointLightComponent = this->AddComponent<PointLightComponent>("pointLight");
        pointLightComponent->SetRange(5.0f);
        pointLightComponent->SetColor({ 1.0f,1.0f,1.0f });
        pointLightComponent->SetIntensity(10.0f);
    }
    std::shared_ptr<PointLightComponent> GetPointLightComponent() const { return pointLightComponent; }

private:
    std::shared_ptr<PointLightComponent> pointLightComponent;
};