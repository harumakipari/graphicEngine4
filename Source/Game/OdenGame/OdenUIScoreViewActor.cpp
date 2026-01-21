#include "pch.h"

#include "OdenUIScoreViewActor.h"

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "Physics/CollisionFunction.h"
#include "UI/FontManager.h"

void OdenUIScoreViewActor::Initialize(const Transform& transform)
{

}

void OdenUIScoreViewActor::Update(float elapsedTime)
{
    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    if (scoreTextUi)
        scoreTextUi->SetWorldPosition({ uiPos.x, uiPos.y });

    auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager");
    if (!actor) return;

    auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor);
    int currentScore = static_cast<int>(gameManager->GetTotalScore());

    if (currentScore > prevScore)
    {
        popupTimer = 0.3f; // ポップアップ時間
    }

    prevScore = currentScore;

    popupTimer -= elapsedTime;
    float scale = (popupTimer > 0.0f) ? 1.3f : 1.0f;

    scoreTextUi->SetScale({ scale, scale });
    //scoreTextUi->SetText(L"Score:" + std::to_wstring(currentScore));
    scoreTextUi->SetText(std::to_wstring(currentScore));

    // 総合スコアを加算する
}

// フォントをセットする
void OdenUIScoreViewActor::SetFontAndMakeTextComponent()
{
    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreTextUi->SetWorldPosition({ 67, 965 });
    scoreTextUi->SetScale({ 1.0f, 1.0f });
    scoreTextUi->SetPivot({ 0.5f,0.5f });

    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);
}
