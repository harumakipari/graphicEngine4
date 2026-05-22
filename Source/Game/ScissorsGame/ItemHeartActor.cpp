#include "pch.h"
#include "ItemHeartActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"

void ItemHeartActor::Initialize(const Transform& transform)
{
    std::string parentName = "ItemHeartActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/ItemHeartModel.gltf", false, true);
    skeletalMeshComponent->SetIsCastShadow(false);

    auto boxComponent = this->AddComponent<class BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    size.x *= 1.3f;
    size.z *= 1.3f;
    float  mass = 0.0f;
    boxComponent->SetBoxExtent({1.5f,1.5f,1.5f});
    boxComponent->SetRelativeLocationDirect({ 0.0f,size.y * 0.5f ,0.0f });
    boxComponent->SetMass(mass);
    boxComponent->SetLayer(CollisionLayer::Item);
    boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->SetResponseToLayer(CollisionLayer::PlayerWeapon, CollisionComponent::CollisionResponse::Trigger);
    boxComponent->Initialize();
    boxComponent->SetOnHitCallback(
        [this](CollisionComponent* self, CollisionComponent* other)
        {
            if (other->GetCollisionLayer() & CollisionHelper::ToBit(CollisionLayer::Player))
            {
                UseItem();
            }
            if (other->GetCollisionLayer() & CollisionHelper::ToBit(CollisionLayer::PlayerWeapon))
            {
                if (auto player = dynamic_cast<ScissorsPlayer1*>(other->GetOwner()))
                {
                    if (player->GetStateMachine()->GetStateName() == "Dash")
                    {// ダッシュ中なら
                        UseItem();
                    }
                }
            }

        }
    );

    elapsedTime = 0.0f;
    itemState = ItemState::Preparing;
}

void ItemHeartActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    elapsedTime += deltaTime;

    switch (itemState)
    {
    case ItemState::Falling:
    {
        velocity.y -= gravity * deltaTime;
        pos.x += velocity.x * deltaTime;
        pos.y += velocity.y * deltaTime;
        pos.z += velocity.z * deltaTime;

        if (pos.y <= groundY)
        {
            pos.y = groundY;
            itemState = ItemState::Waiting;
            elapsedTime = 0.0f;
            basePosition = pos;
        }
        SetPosition(pos);
    }

    break;
    case ItemState::Waiting:
        // 待機時の動き
        UpdateWaiting(deltaTime);
        break;
    case ItemState::Used:
    {
        auto pos = GetPosition();

        // 上に上昇
        pos.y += deltaTime * 3.0f;

        SetPosition(pos);

        // 徐々に縮小
        float scale = std::max<float>(0.0f, 1.0f - elapsedTime * 2.0f);
        SetScale({ scale,scale,scale });

        // 回転
        auto rot = GetEulerRotation();
        rot.y += deltaTime * 360.0f;
        SetEulerRotation(rot);

        if (elapsedTime >= 0.5f)
        {
            MarkPendingKill();
        }
    }
    case ItemState::Hide:
        skeletalMeshComponent->SetIsVisible(false);
        skeletalMeshComponent->SetIsCastShadow(false);
    break;
    }
}

void ItemHeartActor::DrawImGuiDetails()
{

}

// アイテムを使用する
void ItemHeartActor::UseItem()
{
    if (itemState == ItemState::Used)
    {
        return;
    }

    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        return;
    }
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/hpUp.wav", 0.8f);

    player->RecoverHp(2);
    itemState = ItemState::Used;
    elapsedTime = 0.0f;
}

// アイテム待機中の動き
void ItemHeartActor::UpdateWaiting(float deltaTime)
{
    elapsedTime += deltaTime;

    DirectX::XMFLOAT3 pos = basePosition;
    // 上下ふわふわ
    pos.y += sinf(elapsedTime * 3.0f) * 0.15f;

    SetPosition(pos);
}

void ItemHeartActor::LaunchTo(const DirectX::XMFLOAT3& targetPos)
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

    itemState = ItemState::Falling;
}

void ItemHeartActor::HideItemVisual()
{
    itemState = ItemState::Hide;
}