#include "pch.h"
#include "OdenResultScoreActor.h"
#include <magic_enum.hpp>

#include "OdenManagers/OdenGameManager.h"
#include "OdenGameSession.h"
#include "Engine/Scene/Scene.h"
#include "UI/FontManager.h"

void OdenResultScoreActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();

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

            // ここで配置位置を決めて生成する

            float x = globalIndex * 3.0f; // 横に並べる      
            float y = 1.0f;                      // 高さ固定
            Transform ingredientTr(DirectX::XMFLOAT3{ x,y,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
            auto ingredientActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultIngredientActor>("OdenResultIngredient", ingredientTr, ingredientName);
            resultIngredients.push_back(ingredientActor);
            globalIndex++;
        }
    }

    displayScore = 0;

}

void OdenResultScoreActor::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);

    auto& session = OdenGameSession::Instance();

    float score = session.totalScore;
    auto& logs = session.submitLogs;
    auto& counts = session.ingredientCount;


    // 総合スコアを表示する
    if (scoreTextUi)
    {
        scoreTextUi->SetText(std::to_wstring(static_cast<int>(displayScore)));
        scoreTextUi->SetWorldPosition({ baseScorePos.x,baseScorePos.y + popupOffsetY });
    }
    // 総合スコアを表示する
    if (scoreBackTextUi)
    {
        scoreBackTextUi->SetText(std::to_wstring(static_cast<int>(displayScore)));
        scoreBackTextUi->SetWorldPosition({ baseScorePos.x,baseScorePos.y + popupOffsetY });
    }


    // 食材の順番登場
    if (spawnIndex < resultIngredients.size())
    {
        spawnTimer += deltaTime;
        if (spawnTimer >= nextSpawnDelay)
        {
            resultIngredients[spawnIndex]->AppearIngredient();

            int addScore = 100; // 仮のスコア加算値
            AddScore(addScore);

            spawnTimer = 0.0f;
            ++spawnIndex;
        }
    }
}

// フォントをセットする
void OdenResultScoreActor::SetFontAndMakeTextComponent()
{
    baseScorePos = { 1980.0f * 0.5f,1080.0f * 0.5f };

    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");

    scoreTextUi->SetWorldPosition(baseScorePos);
    scoreTextUi->SetScale({ 3.0f, 3.0f });
    scoreTextUi->SetPivot({ 0.5f,0.5f });
    scoreTextUi->zOrder = 20;

    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);

    scoreBackTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreBackTextUi->SetWorldPosition(baseScorePos);
    scoreBackTextUi->SetScale({ 3.5f, 3.5f });
    scoreBackTextUi->SetPivot({ 0.5f,0.5f });
    scoreBackTextUi->SetColor(CoreColor::Black);
    scoreBackTextUi->zOrder = 15;
    uiManager->Add(scoreBackTextUi);

}

// 演出のためにスコアを加算する
void OdenResultScoreActor::AddScore(int add)
{
    displayScore += add;

    popupOffsetY = 0.0f;

    TestEasingHandler handler;
    handler.AddEasing(
        TestEaseType::OutElastic,
        0.0f,
        20.0f,
        0.3f
    );

    handler.AddEasing(
        TestEaseType::InQuad,
        20.0f,
        0.0f,
        0.15f
    );

    handler.SetCompletedFunction([this]()
        {
            popupOffsetY = 0.0f;
        });
    PropertyAccessor<float> accessor;
    
    accessor.getter = [this]() { return popupOffsetY; };
    accessor.setter = [this](float t)
        {
            popupOffsetY = t;
        };

    easingRunner->StartHandler(handler, accessor);
}