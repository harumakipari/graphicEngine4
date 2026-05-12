#include "pch.h"
#include "YarnWallActor.h"

#include "Game/Actors/Base/Character.h"


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

    switch (state)
    {
    case WallState::Hidden:
    {
        // 出現タイプ
        if (behavior == WallBehavior::AppearOnce)
        {
            if (elapsedTime >= appearTime)
            {
                state = WallState::Rising;
            }
        }
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
            if (elapsedTime >= hideTime)
            {
                state = WallState::Lowering;
            }
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
    }
    else
    {
        state = WallState::Visible;

        pos.y = visibleY;
        SetPosition(pos);
    }
}

