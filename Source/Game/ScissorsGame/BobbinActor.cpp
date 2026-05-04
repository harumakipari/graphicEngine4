#include "pch.h"
#include "BobbinActor.h"

#include "EnemyBase.h"
#include "RabbitBossEnemy.h"
#include "ScissorsPlayer1.h"
#include "WaveManagaer.h"
#include "Engine/Scene/Scene.h"

void BobbinActor::Initialize(const Transform& transform)
{
    std::string parentName = "BobbinActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/BobbinModel.glb", false, true);


    auto boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = MathHelper::MultiplyF3XF3(skeletalMeshComponent->GetModelSize(), transform.GetScale());
    float  mass = 0.0f;
    boxComponent->SetBoxExtent(size);
    boxComponent->SetRelativeLocationDirect({ 0.0f,size.y * 0.5f ,0.0f });
    boxComponent->SetMass(mass);
    boxComponent->SetLayer(CollisionLayer::Bobbin);
    //boxComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    boxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->Initialize();
    boxComponent->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            uint32_t mask = CollisionHelper::ToBit(CollisionLayer::PlayerWeapon);
            if (other->GetCollisionLayer() != mask)// ダッシュの当たり判定じゃなかったら、
                return;

            auto player = dynamic_cast<ScissorsPlayer1*>(other->GetOwner());
            if (!player) return;

            if (player->GetStateMachine()->GetStateName() == "Dash")
            {// playerが突進中だったら
                UseBobbin();
            }
        }
    );
    bobbinState = BobbinState::CoolDown;
}

void BobbinActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 center = GetPosition();

    switch (bobbinState)
    {
    case BobbinState::CoolDown:
        cooldownTimer -= deltaTime;
        if (cooldownTimer < 0.0f)
        {
            bobbinState = BobbinState::Charging;
        }
        break;
    case BobbinState::Charging:
    {
        chargeTimer += deltaTime;

        float t = chargeTimer / chargeTime;
        t = std::clamp(t, 0.0f, 1.0f);

        currentRadius = maxRadius * t;
    }
    break;
    case BobbinState::Fired:
        ApplyToEnemies(center);
        Reset();
        break;
    }

    // ボビンの当たり判定をデバッグ表示
    DebugRender::DrawSphere(center, currentRadius, { 1,0,0.5f,1 }, 0, true);

}


void BobbinActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("ボビンを使用する")))
    {
        UseBobbin();
    }
    ImGui::DragFloat(U8("広がる最大半径"), &maxRadius, 0.5f);
    ImGui::DragFloat(U8("クールタイム"), &cooldownInterval, 0.5f);
    ImGui::DragFloat(U8("maxになるまでにかかる時間"), &chargeTime, 0.5f);

#endif
}

// ボビンを使用する
void BobbinActor::UseBobbin()
{
    bobbinState = BobbinState::Fired;
}

// ボビンをリセットする
void BobbinActor::Reset()
{
    currentRadius = 0.0f;
    chargeTimer = 0.0f;
    hitEnemies.clear();
    bobbinState = BobbinState::CoolDown;
    cooldownTimer = cooldownInterval;
}

// 敵を玉止めする
void BobbinActor::ApplyToEnemies(const DirectX::XMFLOAT3 center)
{
    std::vector<std::shared_ptr<ITieable>> candidates;

    auto waveManager = GetOwnerScene()->GetActorManager()->GetActorOfType<WaveManager>();

    // 敵を集める
    for (auto& w : waveManager->aliveEnemies)
    {
        if (auto e = w.lock())
        {
            if (!e->IsDead())
            {
                candidates.push_back(e);
            }
        }
    }

    auto boss = GetOwnerScene()->GetActorManager()->GetActorOfType<RabbitBossEnemyActor>();
    if (boss)
    {// ボスがいたら
        candidates.push_back(boss);
    }

    for (auto e : candidates)
    {
        auto actor = dynamic_cast<Actor*>(e.get());
        if (!actor) continue;

        auto enemyPos = actor->GetPosition();

        float dx = enemyPos.x - center.x;
        float dz = enemyPos.z - center.z;

        float distanceSq = dx * dx + dz * dz;
        float radiusSq = currentRadius * currentRadius;

        if (distanceSq <= radiusSq)
        {
            if (e->IsTied())
            {
                // すでに玉止めされている → 死亡
                if (auto enemy = dynamic_cast<EnemyBase*>(actor))
                {
                    enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
                    enemy->CallDeath(false);
                }
            }
            else
            {
                // 玉止めされていない → 玉止め
                e->OnTied();
            }
        }
    }
}
