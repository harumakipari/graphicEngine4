#include "pch.h"
#include "BonusUiActor.h"

#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void BonusUiActor::Initialize(const Transform& transform)
{
    auto uiManager = GetOwnerScene()->GetUIManager();
    // ボーナスのUIコンポーネントを追加
    {
        bonusUiComponent = std::make_unique<UIImageComponent>("./Data/Textures/ScissorsUI/Bonus.png", "Bonus");
        bonusUiComponent->SetWorldPosition({ 0.0f, 0.0f });
        bonusUiComponent->SetSize({ 130.0f, 90.0f });
        bonusUiComponent->SetPivot({ 0.5f, 0.5f });
        //bonusUiComponent->SetVisible(false);
        uiManager->Add(bonusUiComponent);
    }
}

void BonusUiActor::Update(float deltaTime)
{
    DirectX::XMFLOAT3 pos = GetPosition();
    DirectX::XMFLOAT2 uiPos = WorldToUI(pos);

    bonusUiComponent->SetWorldPosition(uiPos);

    elapsedTime += deltaTime;

    if (elapsedTime > 3.0f)
    {
        MarkPendingKill();
        bonusUiComponent->MarkPendingKill();
    }
}

