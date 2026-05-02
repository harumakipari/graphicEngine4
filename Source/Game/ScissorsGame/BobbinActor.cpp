#include "pch.h"
#include "BobbinActor.h"

#include "EnemyBase.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void BobbinActor::Initialize(const Transform& transform)
{
    std::string parentName = "BobbinActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/BobbinModel.glb", false, true);


    auto boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    float  mass = 0.0f;
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,size.y * 0.5f ,0.0f });
    boxComponent->SetMass(mass);
    boxComponent->SetLayer(CollisionLayer::RibbonWall);
    boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::RibbonWall, CollisionComponent::CollisionResponse::Block);
    boxComponent->Initialize();
    boxComponent->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {

        }
    );


}

void BobbinActor::Update(float deltaTime)
{
    currentRadius += expandSpeed * deltaTime;

    currentRadius = std::min<float>(currentRadius, maxRadius);

    DirectX::XMFLOAT3 center = GetPosition();

    // playerの当たり判定をデバッグ表示
    DebugRender::DrawSphere(center, currentRadius, { 1,0,0.5f,1 }, 0, true);

    if (!isActivated) return;

    std::vector<std::shared_ptr<EnemyBase>> candidates;

    auto waveManager = GetOwnerScene()->GetActorManager()->GetActorOfType<WaveManager>();

    // 敵を集める
    for (auto& w : waveManager->aliveEnemies)
    {
        if (auto e = w.lock())
        {
            if (!e->IsDead() && e->GetState() == EnemyBase::YarnState::Active)
            {
                candidates.push_back(e);
            }
        }
    }



    for (auto e : candidates)
    {
        DirectX::XMFLOAT3 enemyPos = e->GetPosition();
        float dx = enemyPos.x - center.x;
        float dz = enemyPos.z - center.z;

        float distanceSq = dx * dx + dz * dz;
        float radiusSq = currentRadius * currentRadius;

        if (distanceSq <= radiusSq)
        {
            e->ForceTied();
        }
    }

}
