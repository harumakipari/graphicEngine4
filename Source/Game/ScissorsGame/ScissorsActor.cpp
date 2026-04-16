#include "pch.h"
#include "ScissorsActor.h"

#include "Engine/Scene/SceneBase.h"

#include "ScissorsPlayer.h"
#include "YarnEnemyActor.h"

void ScissorsActor::Initialize(const Transform& transform)
{
    std::string parentName = "ScissorsActor";

    // ハサミの片方
    meshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    meshComponent->SetModel("./Data/TeamModels/Scissors/scissors.glb", false, false);
    meshComponent->SetIsVisible(false); // 最初は見えない（プレイヤーの手に持ってる想定）

    sphereComponent = AddComponent<SphereComponent>("sphere", parentName);
    sphereComponent->SetRadius(attackRadius);
    sphereComponent->SetLayer(CollisionLayer::Player); 
    sphereComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Trigger);
    sphereComponent->Initialize();
}

void ScissorsActor::Update(float deltaTime)
{
    if (!owner) return;

    switch (state)
    {
    case State::Equipped:
    {
        // プレイヤーに追従
        auto playerPos = owner->GetPosition();

        DirectX::XMFLOAT3 offset = { 0.3f, 0.0f, 0.0f }; // 手の位置（仮）
        SetPosition({
            playerPos.x + offset.x,
            playerPos.y + offset.y,
            playerPos.z + offset.z
            });
        break;
    }

    case State::Pulling:
    {
        auto playerPos = owner->GetPosition();
        auto pos = GetPosition();

        float pullSpeed = 5.0f; // ←調整ポイント

        float dist = MathHelper::Distance(pos, playerPos);

        // 近づくほど速く
        pullSpeed = std::max<float>(5.0f, dist * 5.0f);

        pos.x += (playerPos.x - pos.x) * pullSpeed * deltaTime;
        pos.y += (playerPos.y - pos.y) * pullSpeed * deltaTime;
        pos.z += (playerPos.z - pos.z) * pullSpeed * deltaTime;


        SetPosition(pos);

        // 近づいたら回収
        if (dist < 0.3f)
        {
            owner->OnScissorsReturned(this);
        }

        break;
    }
    case State::Attacking:
    {
        attackTimer += deltaTime;

        auto playerPos = owner->GetPosition();
        DirectX::XMFLOAT3 forward = owner->GetForward();

        SetPosition({
            playerPos.x + forward.x * 1.0f,
            playerPos.y,
            playerPos.z + forward.z * 1.0f
            });

        // 一定時間攻撃判定を維持
        if (attackTimer >= attackDuration)
        {
            PickUp();
        }

        break;
    }

    case State::Thrown:
    {
        auto pos = GetPosition();

        pos.x += velocity.x * deltaTime;
        pos.y += velocity.y * deltaTime;
        pos.z += velocity.z * deltaTime;

        SetPosition(pos);

        float dist = MathHelper::Distance(pos, targetPos);

        if (dist < 0.2f) // ちょい余裕持たせる
        {
            SetPosition(targetPos); // ピタッと合わせる
            velocity = { 0,0,0 };

            state = State::Dropped; // 地面に落ちた扱い
        }

        break;
    }
    case State::Dropped:
        // 何もしない（地面に置いてるだけ）
        break;
    }


    DebugRender::DrawSphere(
        GetPosition(),
        attackRadius,
        {1.0f, 0.0f, 0.0f, 1.0f}
    );
}

void ScissorsActor::OnHit(std::pair<CollisionComponent*, CollisionComponent*> hitPair)
{
    auto otherCollision = hitPair.second;
    auto other = otherCollision->GetOwner();

    auto enemy = dynamic_cast<YarnEnemyActor*>(other);
    if (!enemy) return;

    if (hitEnemies.contains(enemy)) return;

    int damage = 0;

    if (state == State::Pulling)
    {
        damage = 2;
    }
    else if (state == State::Attacking)
    {
        damage = (owner->GetScissorsCount() == 2) ? 2 : 1;
    }
    else
    {
        return;
    }

    hitEnemies.insert(enemy);
    enemy->TakeDamage(damage);
}

void ScissorsActor::Drop(const DirectX::XMFLOAT3& pos)
{
    state = State::Dropped;
    SetPosition(pos);

    meshComponent->SetIsVisible(true);
}

void ScissorsActor::PickUp()
{
    state = State::Equipped;
    meshComponent->SetIsVisible(false);
}

// ハサミを攻撃に使う
void ScissorsActor::StartAttack()
{
    state = State::Attacking;
    hitEnemies.clear();

    attackTimer = 0.0f;
}

// ハサミを投げる
void ScissorsActor::Throw(DirectX::XMFLOAT3 dir, float power)
{
    state = State::Thrown;

    float maxThrowDistance = 10.0f;
    float distance = power * maxThrowDistance;

    auto start = GetPosition();
    // まず理想位置
    DirectX::XMFLOAT3 desired =
    {
        start.x + dir.x * distance,
        start.y + dir.y * distance,
        start.z + dir.z * distance
    };

    // clamp
    float stageMinX = -0.5f;
    float stageMaxX = 12.5f;
    float stageMinZ = -0.5f;
    float stageMaxZ = 12.5f;

    desired.x = std::clamp(desired.x, stageMinX, stageMaxX);
    desired.z = std::clamp(desired.z, stageMinZ, stageMaxZ);

    targetPos = desired;

    // ここが重要：velocityをtarget基準で作る
    DirectX::XMFLOAT3 dirToTarget =
    {
        targetPos.x - start.x,
        targetPos.y - start.y,
        targetPos.z - start.z
    };

    float len = sqrt(dirToTarget.x * dirToTarget.x + dirToTarget.z * dirToTarget.z);

    if (len > 0.001f)
    {
        dirToTarget.x /= len;
        dirToTarget.z /= len;
    }

    velocity.x = dirToTarget.x * throwSpeed;
    velocity.y = dirToTarget.y * throwSpeed;
    velocity.z = dirToTarget.z * throwSpeed;

    meshComponent->SetIsVisible(true);
}

// ハサミを引き寄せる
void ScissorsActor::StartPull(const DirectX::XMFLOAT3& target)
{
    state = State::Pulling;
    hitEnemies.clear();
}