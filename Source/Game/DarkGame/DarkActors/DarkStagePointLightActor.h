#pragma once
#include "Core/Actor.h"
#include "Components/Render/PointLightComponent.h"

class DarkStagePointLightActor :public Actor
{
public:
    DarkStagePointLightActor(const std::string& actorName) :Actor(actorName) {}
    virtual ~DarkStagePointLightActor() = default;
    void Initialize(const Transform& transform)override;

    // ポイントライトのデータを一括で設定する関数
    void SetPointLightData(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color, float intensity, float range);

    std::shared_ptr<PointLightComponent> GetPointLightComponent() const { return pointLightComponent; }
private:
    std::shared_ptr<PointLightComponent> pointLightComponent;
    std::shared_ptr<SkeletalMeshComponent> sphereMeshComponent;
};