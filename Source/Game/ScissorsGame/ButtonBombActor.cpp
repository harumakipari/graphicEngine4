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
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameBlinkPS";

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
                if (other->GetCollisionLayer() & mask)
                {// 敵かplayerが当たったら、爆発する
                    Explode();
                }
            });
    }


    // 登場エフェクト用のコンポーネントを追加
    bombEffectComponent = this->AddComponent<class ParticleComponent>("bombEffect", parentName);
    bombEffectComponent->Load("./Data/Effect/Files/ScissorsGameBombEffect.json");


}

void ButtonBombActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    elapsedTime += deltaTime;

    switch (bombState)
    {
    case BombState::Falling:
    {
        velocity.y -= gravity * deltaTime;
        pos.x += velocity.x * deltaTime; 
        pos.y += velocity.y * deltaTime;
        pos.z += velocity.z * deltaTime; 

        if (pos.y <= groundY)
        {
            pos.y = groundY;
            bombState = BombState::Waiting;
            elapsedTime = 0.0f;
        }
        SetPosition(pos);
    }

    break;
    case BombState::Waiting:
        if (elapsedTime >= blinkDelay)
        {
            bombState = BombState::Blinking;
            elapsedTime = 0.0f;
        }
        break;
    case BombState::Blinking:
        // 点滅
        // オレンジ固定
        skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 0.89f,0.53f,0.01f,1 };
        // 点滅ON/OFFだけ送る
        skeletalMeshComponent->plusAlphaCBuffer->data.flashValue = (sinf(elapsedTime * 20.0f) + 1.0f) * 0.5f;

        if (elapsedTime >= explodeDelay)
        {
            Explode();
        }

        break;
    case BombState::Exploded:
        if (elapsedTime>=0.5f)
        {
            MarkPendingKill();
        }
        break;
    }

    // 爆発範囲のデバック描画
    DebugRender::DrawSphere(pos, explodeRange, { 1,1,0.5f,1 }, 0, true);
}

void ButtonBombActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("爆発する")))
    {
        hasExploded = false;
        Explode();
    }
    if (ImGui::Button(U8("リセットする")))
    {
        hasExploded = false;
    }


#endif
}

// 爆発処理
void ButtonBombActor::Explode()
{
    if (hasExploded)
    {// 爆発したことがあったら、
        return;
    }
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/bomb_explosion.wav", 0.8f);


    hasExploded = true;
    bombState = BombState::Exploded;
    elapsedTime = 0.0f;

    skeletalMeshComponent->SetIsVisible(false);

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

// ボス位置から爆弾位置まで発射する処理
void ButtonBombActor::LaunchTo(const DirectX::XMFLOAT3& targetPos)
{
    auto start = GetPosition();

    DirectX::XMFLOAT3 diff;
    diff.x = targetPos.x - start.x;
    diff.y = targetPos.y - start.y;
    diff.z = targetPos.z - start.z;

    float t = 0.8f;        // ← 落ちる時間

    velocity.x = diff.x / t;
    velocity.z = diff.z / t;

    velocity.y = (diff.y + 0.5f * gravity * t * t) / t;

    bombState = BombState::Falling;
}

// 爆発エフェクトを再生する
void ButtonBombActor::PlayBombEffect(DirectX::XMFLOAT3 pos)
{
    if (bombEffectComponent)
    {
        bombEffectComponent->SetWorldLocationDirect(pos);
        bombEffectComponent->UpdateComponentToWorld(); // これ入れないと最初に呼ばれる時に位置がずれる
        XMFLOAT3 position = bombEffectComponent->GetComponentLocation();
        XMFLOAT3 rotation = bombEffectComponent->GetComponentEulerRotation();
        EffectManager::EmitParticle(bombEffectComponent->GetEffectHandle(), position, rotation);

    }
}