#include "pch.h"
#include "YarnWallActor.h"

#include "Engine/Scene/Scene.h"
#include "Game/Actors/Base/Character.h"
#include "WaveManagaer.h"


void YarnWallActor::Initialize(const Transform& transform)
{
    std::string parentName = "YarnWallActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetRelativeLocationDirect({ 0, 0.0f, 0 });
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/YarnWallModel.gltf");
    //skeletalMeshComponent->SetModel("./Data/TeamModels/Item/YarnWallModel.glb");


    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    size.z += 1.0f;
    size.x += 1.5f;
    // 反射用の当たり判定
    redirectCollisionComponent = this->AddComponent<BoxComponent>("redirectCollisionComponent", parentName);
    redirectCollisionComponent->SetBoxExtent(size);
    redirectCollisionComponent->SetMass(0.0f);
    redirectCollisionComponent->SetCollisionOffsetY(0.3f);
    redirectCollisionComponent->SetLayer(CollisionLayer::EnemyRedirect);
    redirectCollisionComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
    redirectCollisionComponent->Initialize();
}

void YarnWallActor::Update(float deltaTime)
{
    elapsedTime += deltaTime;

    auto pos = GetPosition();
    auto waveManager = GetOwnerScene()->GetActorManager()->GetActorOfType<WaveManager>();

    if (waveManager && !triggered)
    {
        if (waveManager->GetCurrentWave() >= triggerWave)
        {
            triggered = true;

            if (behavior == WallBehavior::AppearOnce)
            {
                state = WallState::Rising;
            }
            else if (behavior == WallBehavior::HideOnce)
            {
                state = WallState::Lowering;
            }
        }
    }

    switch (state)
    {
    case WallState::Hidden:
    {
#if 0
        // 出現タイプ
        if (behavior == WallBehavior::AppearOnce)
        {
            if (elapsedTime >= appearTime)
            {
                state = WallState::Rising;
            }
        }
#endif // 0
    }
    break;

    case WallState::Rising:
    {
        pos.y += moveSpeed * deltaTime;

        if (pos.y >= visibleY)
        {
            pos.y = visibleY;
            state = WallState::Visible;
        }

        SetPosition(pos);

    }
    break;

    case WallState::Visible:
    {
        // 隠れるタイプ
        if (behavior == WallBehavior::HideOnce)
        {
#if 0
            if (elapsedTime >= hideTime)
            {
                state = WallState::Lowering;
            }
#endif // 0
        }
    }
    break;

    case WallState::Lowering:
    {
        pos.y -= moveSpeed * deltaTime;

        if (pos.y <= hiddenY)
        {
            pos.y = hiddenY;
            state = WallState::Hidden;
        }

        SetPosition(pos);
    }
    break;
    }
}

// AppearTimeを設定したあとに呼び出す関数
void YarnWallActor::SetUp()
{
    DirectX::XMFLOAT3 pos = GetPosition();
    // 最初から隠れているタイプ
    if (behavior == WallBehavior::AppearOnce)
    {
        state = WallState::Hidden;

        pos.y = hiddenY;
        SetPosition(pos);

        //skeletalMeshComponent->SetIsVisible(false);
        //redirectCollisionComponent->DisableCollision();

    }
    else
    {
        state = WallState::Visible;

        pos.y = visibleY;
        SetPosition(pos);

        //skeletalMeshComponent->SetIsVisible(true);
        //redirectCollisionComponent->EnableCollision();

    }
}

