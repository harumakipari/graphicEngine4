#include "pch.h"
#include "NeedleActor.h"

#include "ScissorsPlayer1.h"

void NeedleActor::Initialize(const Transform& transform)
{
    std::string parentName = "needleActor";
    mesh = AddComponent<SkeletalMeshComponent>(parentName);
    mesh->SetModel("./Data/TeamModels/Enemy/NeedlePin.glb", false, true);

    collision = AddComponent<SphereComponent>("collision", parentName);
    float radius = 0.5f;
    float height = radius;
    float mass = 100.0f;
    collision->SetRadius(radius);
    collision->SetMass(mass);
    collision->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
    collision->SetLayer(CollisionLayer::Projectile);
    collision->SetStatic(true); // ìÆÇ©Ç≥Ç»Ç¢
    collision->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    collision->SetCollisionOffsetY(height * 0.5f);
    collision->Initialize();
    collision->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            auto player = dynamic_cast<ScissorsPlayer1*>(other->GetOwner());
            if (!player) return;
            Logger::Log(U8("ÉvÉåÉCÉÑÅ[Ç…êjÇ™ìñÇΩÇÈ"));
        }
    );
}

void NeedleActor::Update(float deltaTime)
{
    auto pos = GetPosition();
    pos.x += velocity.x * speed * deltaTime;
    pos.z += velocity.z * speed * deltaTime;
    SetPosition(pos);
}
