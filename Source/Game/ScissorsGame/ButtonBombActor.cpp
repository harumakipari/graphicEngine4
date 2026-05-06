#include "pch.h"

#include "ButtonBombActor.h"

#include "Engine/Scene/SceneBase.h"
#include "ScissorsPlayer1.h"
#include "EnemyBase.h"
#include "RabbitBossEnemy.h"

void ButtonBombActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/BombModel.glb", false, true);

    // 当たり判定
    {
        std::shared_ptr<SphereComponent> sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        float radius = size.x * 0.5f;
        float height = size.y;
        float mass = 60.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(mass);
        sphereComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereComponent->SetLayer(CollisionLayer::Bomb);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetCollisionOffsetY(height * 0.5f);
        sphereComponent->Initialize();
        sphereComponent->SetOnHitCallback(
            [this](CollisionComponent* self, CollisionComponent* other)
            {
                uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Enemy) | CollisionHelper::ToBit(CollisionLayer::Player);
                if (other->GetCollisionLayer() == mask)
                {// 敵かplayerが当たったら、爆発する
                    Explode();
                }
            });

    }


    // 登場エフェクト用のコンポーネントを追加
    bombEffectComponent = this->AddComponent<class ParticleComponent>("bombEffect",parentName);
    bombEffectComponent->Load("./Data/Effect/Files/ScissorsGameBombEffect.json");


}

void ButtonBombActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();

    // 爆発範囲のデバック描画
    DebugRender::DrawSphere(pos, explodeRange, { 1,1,0.5f,1 }, 0, true);
}

void ButtonBombActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("爆発する")))
    {
        Explode();
    }

#endif
}

// 爆発処理
void ButtonBombActor::Explode()
{
    DirectX::XMFLOAT3 pos = GetPosition();

    // エフェクトを生成する
    DirectX::XMFLOAT3 bombEffectPos = pos;
    bombEffectPos.y += 0.5f;
    PlayBombEffect(bombEffectPos);

    for (auto enemy : GetOwnerScene()->GetActorManager()->GetActorsOfType<EnemyBase>())
    {
        if (!enemy) continue;

        if (enemy->IsDead())
        {// 敵が死亡している場合は無視
            continue;
        }

        if (auto boss = std::dynamic_pointer_cast<RabbitBossEnemyActor>(enemy))
        {// ボスの時 
            continue; // ボスには攻撃が当たらないようにする
        }

        DirectX::XMFLOAT3 enemyPos = enemy->GetPosition();
        enemyPos.y = 0.0f;
        pos.y = 0.0f;

        float distSq = MathHelper::DistanceSq(enemyPos, pos);


        if (distSq < explodeRange * explodeRange)
        {
            enemy->OnTied();
            enemy->ChangeEnemyState(EnemyBase::YarnState::Dead);
            enemy->CallDeath(false); // 死亡演出開始処理
            Logger::Log(U8("爆弾によって敵が死亡した"));
        }
    }

    // プレイヤーにもダメージ
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (player)
    {

        DirectX::XMFLOAT3 playerPos = player->GetPosition();
        playerPos.y = 0.0f;
        pos.y = 0.0f;


        float distSq = MathHelper::DistanceSq(playerPos, pos);
        if (distSq < explodeRange * explodeRange)
        {
            player->TakeDamage(1);
            Logger::Log(U8("爆弾によってプレイヤーにダメージが入った"));
        }
    }
}

// 爆発エフェクトを再生する
void ButtonBombActor::PlayBombEffect(DirectX::XMFLOAT3 pos)
{
    if (bombEffectComponent)
    {
        bombEffectComponent->SetWorldLocationDirect(pos);
        bombEffectComponent->Play();
    }
}