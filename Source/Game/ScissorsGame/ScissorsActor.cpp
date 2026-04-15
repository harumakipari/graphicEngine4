#include "pch.h"
#include "ScissorsActor.h"

#include "Engine/Scene/SceneBase.h"

#include "ScissorsPlayer.h"

void ScissorsActor::Initialize(const Transform& transform)
{
    std::string parentName = "ScissorsActor";

    // ハサミの片方
    meshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    meshComponent->SetModel("./Data/TeamModels/Scissors/scissors.glb", false, false);
    meshComponent->SetIsVisible(false); // 最初は見えない（プレイヤーの手に持ってる想定）
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

        // 補間で近づける
        pos.x += (playerPos.x - pos.x) * 1.0f * deltaTime;
        pos.y += (playerPos.y - pos.y) * 1.0f * deltaTime;
        pos.z += (playerPos.z - pos.z) * 1.0f * deltaTime;

        SetPosition(pos);

        // 近づいたら回収
        float dist = MathHelper::Distance(pos, playerPos);
        if (dist < 1.0f)
        {
            PickUp();
        }

        break;
    }

    case State::Dropped:
        // 何もしない（地面に置いてるだけ）
        break;
    }
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

void ScissorsActor::StartPull(const DirectX::XMFLOAT3& target)
{
    state = State::Pulling;
}