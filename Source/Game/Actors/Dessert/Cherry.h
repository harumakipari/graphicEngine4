#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"

class Cherry : public Actor
{
public:
    Cherry(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeletalMeshComponent> cherry;
    std::shared_ptr<ParticleComponent> particleComponent;
    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        std::string parentName = "cherry";
        cherry = this->NewSceneComponent<SkeletalMeshComponent>(parentName);
        cherry->SetModel("./Data/Models/cherry_pudding/cherry_zero.glb");

        std::shared_ptr<SphereComponent> sphereComponent = this->NewSceneComponent<SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = cherry->GetModelSize();
        sphereComponent->SetRadius(size.x * 0.5f);
        sphereComponent->SetMass(40.0f);
        sphereComponent->SetLayer(CollisionLayer::Enemy);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->Initialize();

        particleComponent = this->NewSceneComponent<class ParticleComponent>("particleComponent", parentName);
        particleComponent->Load("./Data/Effect/Files/testEffect.json");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
    }

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override
    {
    };

    void Launch(const XMFLOAT3& startPos, const XMFLOAT3& velocity);

private:
    XMFLOAT3 velocity_ = { 0.0f,0.0f,0.0f };
    float gravity_ = -9.8f;
    bool isFlying_ = false;
};
