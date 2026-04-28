#include "pch.h"

#include "ScorePopupActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void ScorePopupActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
    auto uiManager = GetOwnerScene()->GetUIManager();

    lifeTime = lifeTimeDuration;

    for (int i = 0; i < 6; i++) // 6桁くらい確保
    {
        auto digit = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/number.png", "ScoreDigit");

        digit->SetSize({ 45, 60 });
        digit->SetPivot({ 0.5f, 0.5f });

        uiManager->Add(digit);
        scoreDigits.push_back(digit);
    }


}

void ScorePopupActor::Update(float elapsedTime)
{
    lifeTime -= elapsedTime;
    easingRunner->Tick(elapsedTime);

    // ワールド位置
    auto pos = GetPosition();
    pos.y += elapsedTime * 2.0f; // 調整
    SetPosition(pos);

    // UI座標に変換
    DirectX::XMFLOAT2 uiPos = WorldToUI(pos);

    float alpha = lifeTime / lifeTimeDuration;

    float digitSpacing = 45.0f;

    for (int i = 0; i < scoreDigits.size(); i++)
    {
        DirectX::XMFLOAT2 pos = uiPos;

        // 左に並べる
        pos.x -= i * digitSpacing;

        scoreDigits[i]->SetWorldPosition(pos);

        // フェードアウト
        scoreDigits[i]->SetColor(DirectX::XMFLOAT4{ 1,1,1,alpha });
    }

    if (lifeTime <= 0.0f)
    {
        MarkPendingKill();
        return;
    }
}

void ScorePopupActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
#endif
}

// スコアを設定する
void ScorePopupActor::SetScore(int score)
{
    popupScore = score;

    for (int i = 0; i < scoreDigits.size(); i++)
    {
        int d = score % 10;
        score /= 10;

        scoreDigits[i]->SetUV({ 150.0f * d, 0, 150, 200 });
        scoreDigits[i]->SetVisible(true);

        if (score == 0)
        {
            for (int j = i + 1; j < scoreDigits.size(); j++)
            {
                scoreDigits[j]->SetVisible(false);
            }
            break;
        }
    }
}

// スコアを桁ごとに分解する
void ScorePopupActor::UpdateScoreDigits(int score) const
{
    if (score == 0)
    {// スコアが０の時
        scoreDigits[0]->SetUV({ 0.0f, 0.0f, 150.0f, 200.0f });
        scoreDigits[0]->SetVisible(true);

        for (int i = 1; i < scoreDigits.size(); i++)
        {
            scoreDigits[i]->SetVisible(false);
        }
        return;
    }

    for (int i = 0; i < scoreDigits.size(); i++)
    {
        int digit = score % 10;
        score /= 10;

        scoreDigits[i]->SetUV({ 150.0f * digit, 0.0f, 150.0f, 200.0f });

        // スコアがまだあるなら表示
        scoreDigits[i]->SetVisible(true);

        if (score == 0 && i > 0)
        {
            // 上位桁は非表示
            for (int j = i + 1; j < scoreDigits.size(); j++)
            {
                scoreDigits[j]->SetVisible(false);
            }
            break;
        }
    }
}
