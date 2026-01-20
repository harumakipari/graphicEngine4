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



    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            scoreTextUi->SetText(L"Score:" + std::to_wstring(static_cast<int>(gameManager->GetTotalScore())));
        }

        //scoreTextUi->SetText(L"あいうえおお\n");
        //scoreTextUi->SetText(L"sss\nあいうえおかくくけ\nGreat！");
    }
}

// フォントをセットする
void OdenUIScoreViewActor::SetFontAndMakeTextComponent()
{
    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreTextUi->SetWorldPosition({ 67, 965 });
    scoreTextUi->SetScale({ 1.0f, 1.0f });

    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);
}
