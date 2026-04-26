#include "pch.h"

#include "ComboUiActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void ComboUiActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
    auto uiManager = GetOwnerScene()->GetUIManager();

    for (int i = 0; i < 2; i++) // 2桁くらい確保
    {
        auto digit = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/number.png", "ScoreDigit");

        digit->SetSize({ 90, 120 });
        digit->SetPivot({ 0.5f, 0.5f });

        uiManager->Add(digit);
        comboDigits.push_back(digit);
    }
}

void ComboUiActor::Update(float elapsedTime)
{
    easingRunner->Tick(elapsedTime);

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        Logger::Error(U8("ComboUiActorでplayerがnullptrです。"));
        return;
    }

    auto& scoreSystem = player->scoreSystem;

    int combo = scoreSystem.GetCombo();
    UpdateScoreDigits(combo);


    float digitSpacing = 90.0f; // 桁の間隔（調整ポイント）

    for (int i = 0; i < comboDigits.size(); i++)
    {
        DirectX::XMFLOAT2 pos = uiPos;

        // i=0が1の位 → 右端
        // iが増えるほど左へ
        pos.x -= i * digitSpacing;

        comboDigits[i]->SetWorldPosition(pos);
    }
}

void ComboUiActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
#endif
}


// スコアを桁ごとに分解する
void ComboUiActor::UpdateScoreDigits(int combo) const
{
    // 全部非表示
    for (auto& d : comboDigits)
    {
        d->SetVisible(false);
    }

    // 0のとき
    if (combo == 0)
    {
        comboDigits[0]->SetUV({ 0.0f, 0.0f, 150.0f, 200.0f });
        comboDigits[0]->SetVisible(true);
        return;
    }

    // 通常処理
    for (int i = 0; i < comboDigits.size(); i++)
    {
        int digit = combo % 10;
        combo /= 10;

        comboDigits[i]->SetUV({ 150.0f * digit, 0.0f, 150.0f, 200.0f });

        if (i == 0 || combo > 0)
        {
            comboDigits[i]->SetVisible(true);
        }
    }
}
