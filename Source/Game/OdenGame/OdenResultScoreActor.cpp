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

    int globalIndex = 0; // 全体での横並びインデックス

    for (auto& log : session.submitLogs)
    {
        for (int i = 0; i < log.count; ++i)
        {
            std::string ingredientName = std::string(magic_enum::enum_name(log.type));
            if (ingredientName.empty())
            {
                Logger::Log("Warning: SubmitLog has invalid EOdenType");
                continue; // 作らない
            }
            float x = globalIndex * 3.0f; // 横に並べる
            float y = 1.0f;                      // 高さ固定
            Transform ingredientTr(DirectX::XMFLOAT3{ x,y,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            auto ingredientActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultIngredientActor>("OdenResultIngredient", ingredientTr, ingredientName);
            resultIngredients.push_back(ingredientActor);
            globalIndex++;
        }
    }

}

void OdenResultScoreActor::Update(float deltaTime)
{
    auto& session = OdenGameSession::Instance();

    float score = session.totalScore;
    auto& logs = session.submitLogs;
    auto& counts = session.ingredientCount;


    // 総合スコアを表示する
    if (scoreTextUi)
        scoreTextUi->SetText(L"ResultScore:" + std::to_wstring(static_cast<int>(score)));


    // 食材の順番登場
    if (spawnIndex < resultIngredients.size())
    {
        spawnTimer += deltaTime;
        if (spawnTimer >= nextSpawnDelay)
        {
            resultIngredients[spawnIndex]->AppearIngredient();
            // 配置
#if 0
            float x = 100.0f + spawnIndex * 80.0f; // 横に並べる
            float y = 1.0f;                      // 高さ固定
            resultIngredients[spawnIndex]->SetPosition({ x, y, 0.0f });
#endif // 0
            spawnTimer = 0.0f;
            ++spawnIndex;
        }
    }
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
