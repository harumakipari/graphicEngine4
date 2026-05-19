#include "pch.h"
#include "NeedleActor.h"

#include "ScissorsPlayer1.h"

void NeedleActor::Initialize(const Transform& transform)
{
    std::string parentName = "needleActor";
    mesh = AddComponent<SkeletalMeshComponent>(parentName);
    mesh->SetModel("./Data/TeamModels/Enemy/NeedlePin.gltf", false, true);

    hasHit = false; // ヒットしたかどうかのフラグをリセットする

    collision = AddComponent<SphereComponent>("collision", parentName);
    float mass = 100.0f;
    collision->SetRadius(radius);
    collision->SetMass(mass);
    collision->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
    collision->SetLayer(CollisionLayer::Projectile);
    collision->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    collision->SetCollisionOffsetY(radius);
    collision->Initialize();
    collision->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            if (hasHit) return;

            auto player = dynamic_cast<ScissorsPlayer1*>(other->GetOwner());
            if (!player) return;

            hasHit = true;

            Logger::Log(U8("プレイヤーに針が当たる"));
            player->TakeDamage(2);

            // 針を消す
            MarkPendingKill();
        }
    );
}

void NeedleActor::Update(float deltaTime)
{
    if (isStuck)
    {// 地面に付いたら、
        lifeTimer -= deltaTime;
        if (lifeTimer <= 0.0f)
        {
            MarkPendingKill();
        }
        return;
    }

    // 重力
    velocity.y += gravity * deltaTime;

    auto pos = GetPosition();
    pos.x += velocity.x *deltaTime;
    pos.y += velocity.y *deltaTime;
    pos.z += velocity.z *deltaTime;
    SetPosition(pos);

    // 地面判定
    if (pos.y <= 0.0f)
    {
        Stick();
    }

    DebugRender::DrawSphere(collision->GetComponentLocation(), radius, { 1,0.5f,0,1 }, 0, true);
}

// 飛ばす最終目的位置
void NeedleActor::SetTargetPos(const DirectX::XMFLOAT3& targetPos)
{
    this->targetPos = targetPos;

    auto start = GetPosition();

    // 差分
    DirectX::XMFLOAT3 diff = MathHelper::Subtract(targetPos, start);

    // 水平距離
    DirectX::XMFLOAT3 horizontal = { diff.x, 0.0f, diff.z };
    float dist = MathHelper::Length(horizontal);

    // 到達時間
    float time = std::clamp(dist * 0.2f, 0.5f, 1.0f);

    // 初速計算
    velocity.x = horizontal.x / time;
    velocity.z = horizontal.z / time;
    velocity.y = (diff.y - 0.5f * gravity * time * time) / time;
}

// 刺さる処理
void NeedleActor::Stick()
{
    isStuck = true;
    velocity = { 0,0,0 };

    auto pos = GetPosition();
    pos.y = -0.2f; // 地面に固定
    SetPosition(pos);

    lifeTimer = lifeTimeInterval;

    collision->SetStatic(true);
}