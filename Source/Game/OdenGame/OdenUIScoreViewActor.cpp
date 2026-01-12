#include "pch.h"

#include "OdenUIScoreViewActor.h"

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "UI/FontManager.h"

void OdenUIScoreViewActor::Initialize(const Transform& transform)
{
    
}

void OdenUIScoreViewActor::Update(float elapsedTime)
{
    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            scoreTextUi->SetText(L"Score : " + std::to_wstring(static_cast<int>(gameManager->GetTotalScore())));
        }
    }
}

// フォントをセットする
void OdenUIScoreViewActor::SetFontAndMakeTextComponent()
{
    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreTextUi->SetFont(FontManager::GetUIFont());
    scoreTextUi->SetWorldPosition({ 1550, 20 });
    scoreTextUi->SetScale({ 1.0f, 1.0f });

    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);
}
