#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"

// リザルト本アクター
class ResultBookActor :public BookBaseActor
{
public:
    explicit ResultBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

private:

    DirectX::XMFLOAT3 scoreRelaticePosition={0.0f,0.0f,0.f};

    std::shared_ptr<SkeletalMeshComponent> numberModel;

};

