#include "pch.h"
#include "OdenResultScoreActor.h"

#include "OdenManagers/OdenGameManager.h"
#include "OdenGameSession.h"
#include "Engine/Scene/Scene.h"
#include "UI/FontManager.h"

void OdenResultScoreActor::Initialize(const Transform& transform)
{

}

void OdenResultScoreActor::Update(float elapsedTime)
{
    auto& session = OdenGameSession::Instance();

    float score = session.totalScore;
    auto& logs = session.submitLogs;
    auto& counts = session.ingredientCount;


    // 総合スコアを表示する
    if (scoreTextUi)
        scoreTextUi->SetText(L"ResultScore:" + std::to_wstring(static_cast<int>(score)));
}

// フォントをセットする
void OdenResultScoreActor::SetFontAndMakeTextComponent()
{
    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreTextUi->SetWorldPosition({ 1550, 20 });
    scoreTextUi->SetScale({ 1.0f, 1.0f });

    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);
}
