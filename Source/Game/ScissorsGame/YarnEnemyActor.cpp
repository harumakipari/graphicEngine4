#include "pch.h"

#include "YarnEnemyActor.h"

#include "Components/Render/PointLightComponent.h"
#include "Engine/Scene/SceneBase.h"
#include "ScissorsPlayer1.h"

void YarnEnemyActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/enemy.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // emissionPowerの値を大きくして、自己発光の強さを上げてみる
    skeletalMeshComponent->overrideDeferredPipelineName = "deferredFightStage";
    skeletalMeshComponent->plusAlphaCBuffer->data.brightness = 5.0f;
    skeletalMeshComponent->plusAlphaCBuffer->data.saturation = 1.4f;

    // 当たり判定
    {
        std::shared_ptr<SphereComponent> sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = size.x * 0.5f;
        height = size.y;
        mass = 180.0f;
        sphereComponent->SetRadius(radius);
        sphereComponent->SetMass(mass);
        sphereComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereComponent->SetLayer(CollisionLayer::Enemy);
        sphereComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereComponent->SetCollisionOffsetY(height * 0.5f);
        sphereComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

}

void YarnEnemyActor::Update(float deltaTime)
{
    Character::Update(deltaTime);

    switch (enemyType)
    {
    case YarnEnemyType::Static:
        // 何もしない
        break;
    case YarnEnemyType::MoveHorizontal:
        MoveLinear(deltaTime);
        break;
    case YarnEnemyType::MoveVertical:
        MoveLinear(deltaTime);
        break;
    case YarnEnemyType::MoveToCenter:
        MoveToCenter(deltaTime);
        break;
    case YarnEnemyType::WaveHorizontal:
        MoveWaveHorizontal(deltaTime);
        break;
    case YarnEnemyType::WaveVertical:
        MoveWaveVertical(deltaTime);
    case YarnEnemyType::ChasePlayer:
        ChasePlayer(deltaTime);
        break;
    }
}

// 敵の種類を設定する関数
void YarnEnemyActor::SetType(YarnEnemyType type)
{
    enemyType = type;
    switch (enemyType)
    {
    case YarnEnemyType::Static:
        // 何もしない
        break;
    case YarnEnemyType::MoveHorizontal:
        moveDirection = { 1,0,0 };
        break;
    case YarnEnemyType::MoveVertical:
        moveDirection = { 0,0,1 };
        break;
    case YarnEnemyType::WaveHorizontal:
        break;
    case YarnEnemyType::WaveVertical:
        break;
    case YarnEnemyType::ChasePlayer:
        break;
    }
}

void YarnEnemyActor::TakeDamage(int damage)
{
    hp -= damage;
    Logger::Log(U8("敵にダメージ：") + std::to_string(damage));
    if (hp <= 0)
    {
        MarkPendingKill();
    }
}

void YarnEnemyActor::OnHitByDash(ScissorsPlayer1* player)
{
    // ダッシュで当たったときの処理
    TakeDamage(2);
}

// 線形移動の処理
void YarnEnemyActor::MoveLinear(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ステージ端で反転
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;

    if (pos.x < stageMinX || pos.x > stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
    }

    if (pos.z < stageMinZ || pos.z > stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);
    }

    SetPosition(pos);
}

// 中心に向かって移動する処理
void YarnEnemyActor::MoveToCenter(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    DirectX::XMFLOAT3 target;

    if (goingToCenter)
        target = centerPosition;
    else
        target = startPosition;

    DirectX::XMFLOAT3 dir =
    {
        target.x - pos.x,
        0,
        target.z - pos.z
    };

    float length = sqrt(dir.x * dir.x + dir.z * dir.z);

    if (length < reachThreshold)
    {
        goingToCenter = !goingToCenter; // 反転
        return;
    }

    dir.x /= length;
    dir.z /= length;

    pos.x += dir.x * speed * deltaTime;
    pos.z += dir.z * speed * deltaTime;

    SetPosition(pos);
}

// 横に波打ちながら移動する処理
void YarnEnemyActor::MoveWaveHorizontal(float deltaTime)
{
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;


    waveTime += deltaTime;

    auto pos = GetPosition();

    // 前進（基準軸）
    startPosition.x += moveDirection.x * speed * deltaTime;

    // 位置計算（基準＋波）
    pos.x = startPosition.x;
    pos.z = startPosition.z + sin(waveTime * waveFrequency) * waveAmplitude;

    SetPosition(pos);

    if (pos.x < stageMinX || pos.x > stageMaxX)
    {
        moveDirection.x *= -1.0f;
    }

}

// 縦に波打ちながら移動する処理
void YarnEnemyActor::MoveWaveVertical(float deltaTime)
{
    float stageMinX = 1.0f;
    float stageMaxX = 19.5f;
    float stageMinZ = 1.0f;
    float stageMaxZ = 19.5f;
    waveTime += deltaTime;

    auto pos = GetPosition();

    startPosition.z += moveDirection.z * speed * deltaTime;
    pos.z = startPosition.z;
    pos.x = startPosition.x + sin(waveTime * waveFrequency) * waveAmplitude;

    SetPosition(pos);

    if (pos.z < stageMinZ || pos.z > stageMaxZ)
    {
        moveDirection.z *= -1.0f;
    }

}

void YarnEnemyActor::ChasePlayer(float deltaTime)
{
    if (auto player=GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>())
    {
        auto targetPos=player->GetPosition();
        auto pos = GetPosition();

        XMFLOAT3 dir =
        {
            targetPos.x - pos.x,
            0,
            targetPos.z - pos.z
        };

        float length = sqrt(dir.x * dir.x + dir.z * dir.z);

        if (length < 0.01f) return;

        dir.x /= length;
        dir.z /= length;

        pos.x += dir.x * speed * deltaTime;
        pos.z += dir.z * speed * deltaTime;

        float stageMinX = 1.0f;
        float stageMaxX = 19.5f;
        float stageMinZ = 1.0f;
        float stageMaxZ = 19.5f;
        pos.x = std::clamp(pos.x, stageMinX, stageMaxX);
        pos.z = std::clamp(pos.z, stageMinZ, stageMaxZ);

        SetPosition(pos);
    }
}

// 大きい敵
void BigYarnEnemyActor::Initialize(const Transform& transform)
{
    maxHp = 2;
    YarnEnemyActor::Initialize(transform);
}

void BigYarnEnemyActor::OnHitByDash(ScissorsPlayer1* player)
{
    TakeDamage(1);
    // playerのダッシュを止める処理を追加
    if (hp >= maxHp - 1)
        player->StopDash();
}

