#include "pch.h"
#include "OdenResultScoreActor.h"
#include <magic_enum.hpp>

#include "OdenManagers/OdenGameManager.h"
#include "OdenGameSession.h"
#include "Engine/Scene/Scene.h"
#include "UI/FontManager.h"

void OdenResultScoreActor::Initialize(const Transform& transform)
{
    auto& session = OdenGameSession::Instance();

    Logger::Log("===== Result Initialize =====");

    // 総合スコア
    Logger::Log("TotalScore: " + std::to_string(session.totalScore));
    Logger::Log("Satisfaction: " + std::to_string(session.satisfaction));

    // 提出ログ確認
    Logger::Log("SubmitLogs Count: " + std::to_string(session.submitLogs.size()));
    for (size_t i = 0; i < session.submitLogs.size(); ++i)
    {
        const auto& log = session.submitLogs[i];

        Logger::Log(
            "Log[" + std::to_string(i) + "] "
            + "Type: " + std::string(magic_enum::enum_name(log.type))
            + " Count: " + std::to_string(log.count)
            + " Score: " + std::to_string(log.score)
        );
    }

    // 具材カウント確認
    Logger::Log("IngredientCount:");
    for (size_t i = 0; i < static_cast<size_t>(EOdenType::Count); ++i)
    {
        int count = session.ingredientCount[i];
        if (count > 0)
        {
            Logger::Log(
                "  "
                + std::string(magic_enum::enum_name(static_cast<EOdenType>(i)))
                + " : " + std::to_string(count)
            );
        }
    }

    Logger::Log("=============================");

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
