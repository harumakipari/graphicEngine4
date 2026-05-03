#include "pch.h"

#include "YarnEnemyActor.h"

#include "Components/Render/PointLightComponent.h"
#include "Engine/Scene/SceneBase.h"
#include "ScissorsPlayer1.h"
#include "Components/Elastic/ElasticComponent.h"

void YarnEnemyActor::Initialize(const Transform& transform)
{
    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnEnemy.glb", false, true);
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.cpuColor = { 1,1,1,1 };


    // 当たり判定
    {
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(mass);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Enemy);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = 3;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };

    // 星のエフェクトを追加
    starEffectComponent = this->AddComponent<ParticleComponent>("starEffect", parentName);
    starEffectComponent->Load("./Data/Effect/Files/ScissorsGameStarEffect.json");


#if 0
    auto uiManager = GetOwnerScene()->GetUIManager();
    auto ring = std::make_shared<UIRingEffect>("./Data/Textures/ScissorsUI/ring.png");
    ring->SetWorldPosition({ 500, 300 });
    ring->SetSize({ 100,100 });
    uiManager->Add(ring);
    for (int i = 0; i < 8; i++)
    {
        auto star = std::make_shared<UILineEffect>("./Data/Textures/ScissorsUI/star.png", DirectX::XMFLOAT2{ 500, 300 });
        star->SetSize({ 100,100 });
        uiManager->Add(star);
    }
    //auto flash = std::make_shared<UICoreFlashEffect>("./Data/Textures/ScissorsUI/particle.png");
    //flash->SetWorldPosition({ 500, 300 });
    //flash->SetSize({ 100,100 });
    //uiManager->Add(flash);

    //auto gizagiza = std::make_shared<UISpikeEffect>("./Data/Textures/ScissorsUI/gizagiza.png");
    //gizagiza->SetWorldPosition({ 500, 300 });
    //gizagiza->SetSize({ 100,100 });
    //uiManager->Add(gizagiza);

#endif // 0

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
        break;
    case YarnEnemyType::ChasePlayer:
        ChasePlayer(deltaTime);
        break;
    }

    XMFLOAT3 pos = GetPosition();
    DebugRender::DrawSphere(pos, enemyRadius, { 0,1,1,1 }, 0, true);

    ScissorsGameEnemyBase::Update(deltaTime);

}

// 敵の種類を設定する関数
void YarnEnemyActor::SetType(YarnEnemyType type)
{
    enemyType = type;
    switch (enemyType)
    {
    case YarnEnemyType::Static:
        rotationComponent->SetDirection({ 0,0,-1 });

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

    rotationComponent->SetDirection(dir);
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
    rotationComponent->SetDirection(moveDirection);

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
    rotationComponent->SetDirection(moveDirection);

}

void YarnEnemyActor::ChasePlayer(float deltaTime)
{
    if (auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>())
    {
        auto targetPos = player->GetPosition();
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
        rotationComponent->SetDirection(dir);


    }
}

// 大きい敵
void BigYarnEnemyActor::Initialize(const Transform& transform)
{
    maxHp = 2;
    enemyRadius = 1.0f;

    std::string parentName = "SkeletonWarriorMeshComponent";
    Character::Initialize(transform);
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnBigEnemy.glb", false, true);
    skeletalMeshComponent->plusAlphaCBuffer->data.objectType = ObjectType::Enemy;   // オブジェクトの種類を Enemy に設定
    skeletalMeshComponent->plusAlphaCBuffer->data.emissionPower = 6.6f;   // emissionPowerの値を大きくして、自己発光の強さを上げてみる
    skeletalMeshComponent->overrideDeferredPipelineName = "ScissorsGameEnemyPS";
    skeletalMeshComponent->plusAlphaCBuffer->data.brightness = 5.0f;
    skeletalMeshComponent->plusAlphaCBuffer->data.saturation = 1.4f;
    skeletalMeshComponent->SetIsVisible(false);

    elasticMeshComponent = AddComponent<ElasticMeshComponent>(parentName);
    elasticMeshComponent->SetModel("./Data/TeamModels/Enemy/YarnBigEnemy.glb", false, true);
    elasticMeshComponent->Initialize();
    elasticMeshComponent->SetUseMouseInput(false); // マウス入力によって引っ張られない

    // 当たり判定
    {
        sphereCollisionComponent = this->AddComponent<class SphereComponent>("sphereComponent", parentName);
        DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
        radius = enemyRadius;
        height = size.y;
        mass = 180.0f;
        sphereCollisionComponent->SetRadius(radius);
        sphereCollisionComponent->SetMass(mass);
        sphereCollisionComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
        sphereCollisionComponent->SetLayer(CollisionLayer::Enemy);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
        sphereCollisionComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        sphereCollisionComponent->SetCollisionOffsetY(height * 0.5f);
        sphereCollisionComponent->Initialize();
    }

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", parentName);

    // Hpの初期化
    hp = maxHp;

    // 最初の位置を保存
    startPosition = transform.GetLocation();

    // 倒したときのスコア
    scoreData = { 100,0 };

    // 星のエフェクトを追加
    starEffectComponent = this->AddComponent<ParticleComponent>("starEffect", parentName);
    starEffectComponent->Load("./Data/Effect/Files/ScissorsGameStarEffect.json");

    // 倒したときのスコア
    scoreData = { 200,0 };

}

bool BigYarnEnemyActor::OnHitByDash(ScissorsPlayer1* player, int dashDamage)
{
    int prevHp = hp;

    TakeDamage(dashDamage, true);
    // playerのダッシュを止める処理を追加
    if (hp >= maxHp - 1)
    {
        //player->StopDash();

        DirectX::XMFLOAT3 dir =
        {
            player->GetPosition().x - GetPosition().x,
            0,
            player->GetPosition().z - GetPosition().z
        };

        // 正規化（重要）
        float len = sqrt(dir.x * dir.x + dir.z * dir.z);
        if (len > 0.0001f)
        {
            dir.x /= len;
            dir.z /= len;
        }

        // 90度回転（どっち向きかは好み）
        DirectX::XMFLOAT3 rotated =
        {
            -dir.z,  // 左回転
            0,
            dir.x
        };

        //player->RedirectDash(rotated);

    }

    // コントローラーを振動させる
    InputSystem::SetVibration(1.0f, 0.15f);

    // 倒したかどうかを返す
    return (hp <= 0 && prevHp > 0);

}

void BigYarnEnemyActor::DrawImGuiDetails()
{
    if (ImGui::Button(U8("力を加える")))
    {
        elasticMeshComponent->AddImpulse(impulse);
    }

    ImGui::DragFloat3(U8("加える力"), &impulse.x, 0.5f, -3.0f, 3.0f);
}