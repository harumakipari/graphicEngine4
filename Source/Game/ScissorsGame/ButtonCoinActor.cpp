#include "pch.h"
#include "ButtonCoinActor.h"

#include <algorithm>

void ButtonCoinActor::Initialize(const Transform& transform)
{
    std::string parentName = "buttonCoin";
    auto skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Item/NormalButtonCoin.glb");
    skeletalMeshComponent->SetIsCastShadow(false); // 

    particleComponent = AddComponent<ParticleComponent>("particleComponent", parentName);
    particleComponent->Load("./Data/Effect/Files/ScissorsGameCoinAppearEffect.json");

    // 開始位置を設定
    startPos = transform.GetLocation();
}


void ButtonCoinActor::Update(float deltaTime)
{
    if (!startPerform)
    {// 演出開始の合図が出ていなかったら
        return;
    }

    elapsedTime += deltaTime;

    float t = elapsedTime / duration;
    t = std::min<float>(t, 1.0f);

    float yOffset = height * (1.0f - (1.0f - t) * (1.0f - t)); // easeOut

    auto pos = startPos;
    pos.y += yOffset;
    SetPosition(pos);

    float rotation = DirectX::XMConvertToDegrees(DirectX::XM_2PI * 1.0f * t); // 三回転
    SetEulerRotation({ 0.0f,rotation,0.0f });

    if (t>=1.0f)
    {
        MarkPendingKill();
    }

}

void ButtonCoinActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("演出開始")))
    {
        StartPerform();
    }

#endif
}

// 演出開始する
void ButtonCoinActor::StartPerform()
{
    startPerform = true;
    elapsedTime = 0.0f;
    SetPosition(startPos);
    particleComponent->Play();

    // 白く発光してスタート


}