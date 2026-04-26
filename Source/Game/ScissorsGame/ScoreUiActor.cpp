#include "pch.h"

#include "ScoreUiActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void ScoreUiActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
    auto uiManager = GetOwnerScene()->GetUIManager();

    for (int i = 0; i < 6; i++) // 6桁くらい確保
    {
        auto digit = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/number.png", "ScoreDigit");

        digit->SetSize({ 90, 120 });
        digit->SetPivot({ 0.5f, 0.5f });

        uiManager->Add(digit);
        scoreDigits.push_back(digit);
    }

#if 0
    //スコアの裏を描画する
    scoreBackUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/gamestage_score.png", "game_stage_score");
    scoreBackUi->SetWorldPosition({ 0.0f, 0.0f });
    scoreBackUi->SetVisible(true);
    scoreBackUi->SetScale({ 1.0f, 1.0f });

    scoreBackUi->SetWorldAngleDegree(26.4f);

    scoreBackUi->SetSize({ 376, 263 });
    scoreBackUi->SetPivot({ 0.5f,0.5f });
    scoreBackUi->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f });
    uiManager->Add(scoreBackUi);
#endif // 0

}

void ScoreUiActor::Update(float elapsedTime)
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
        Logger::Error(U8("ScoreUiActorでplayerがnullptrです。"));
        return;
    }

    auto& scoreSystem = player->scoreSystem;

    int score = scoreSystem.GetTotalScore();
    int combo = scoreSystem.GetCombo();
    UpdateScoreDigits(score);


    float digitSpacing = 90.0f; // 桁の間隔（調整ポイント）

    for (int i = 0; i < scoreDigits.size(); i++)
    {
        DirectX::XMFLOAT2 pos = uiPos;

        // i=0が1の位 → 右端
        // iが増えるほど左へ
        pos.x -= i * digitSpacing;

        scoreDigits[i]->SetWorldPosition(pos);
    }
}

void ScoreUiActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat2("scoreBackOffset", &scoreBackOffset.x);
    ImGui::DragFloat2("scoreOffset", &scoreOffset.x);
#endif
}


// スコアを桁ごとに分解する
void ScoreUiActor::UpdateScoreDigits(int score) const
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
