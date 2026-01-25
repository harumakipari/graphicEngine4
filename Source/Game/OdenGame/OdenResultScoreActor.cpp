#include "pch.h"
#include "OdenResultScoreActor.h"
#include <magic_enum.hpp>

#include "OdenManagers/OdenGameManager.h"
#include "OdenGameSession.h"
#include "OdenHighScoreData.h"
#include "Engine/Scene/Scene.h"
#include "UI/FontManager.h"

static std::vector<OdenSubmitLog> CreateDebugSubmitLogs()
{
    return {
        { EOdenType::Daikon,     2, 400.0f },
        { EOdenType::Egg,     1, 200.0f },
        { EOdenType::Chikuwa,    2, 300.0f },
        { EOdenType::Konnyaku,   1, 100.0f },
        { EOdenType::Daikon,     1, 400.0f },
        { EOdenType::Cake,   1, 100.0f },
        { EOdenType::Hanpen,   1, 100.0f },
        { EOdenType::Donut,   1, 100.0f },
        { EOdenType::Cake,   1, 100.0f },
        { EOdenType::Shirataki,   1, 100.0f },
        { EOdenType::Kobumusubi,   1, 100.0f },
        { EOdenType::Tsukune,   1, 100.0f },
        { EOdenType::Goboten,   1, 100.0f },
    };
}

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
    int noMissBonus = (session.missCount == 0) ? 500 : 0;
    int feverBonus = session.feverSubmitCount * 100;
    int maxCombo = session.maxCombo;
    Logger::Log(U8("noMissBonus") + std::to_string(noMissBonus));
    Logger::Log(U8("feverBonus") + std::to_string(feverBonus));
    Logger::Log(U8("連続成功人数") + std::to_string(maxCombo));


    Logger::Log("=============================");

    const std::vector<OdenSubmitLog>* logs = nullptr;

#if _DEBUG
    static bool useDebug = false; // ← ImGui で切り替えてもいい
    if (useDebug)
    {
        static std::vector<OdenSubmitLog> debugLogs = CreateDebugSubmitLogs();
        logs = &debugLogs;
    }
    else
#endif
    {
        logs = &session.submitLogs;
    }



    int globalIndex = 0; // 全体での横並びインデックス

    for (auto& log : *logs)
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

            //float x = 1.2f + globalIndex * 2.4f; // 横に並べる      
            float x = 1.2f + globalIndex * 5.0f; // 横に並べる      
            float y = 6.723f;                      // 高さ固定
            float z = -5.506f;
            Transform ingredientTr(DirectX::XMFLOAT3{ x,y,z }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 2.5f,2.5f,2.5f });
            auto ingredientActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultIngredientActor>("OdenResultIngredient", ingredientTr, ingredientName);
            ingredientActor->SetFeverModeIngredient(log.wasFever);// フィーバー中に提出された具材かどうかを設定する
            resultIngredients.push_back(ingredientActor);
            globalIndex++;
        }
    }

    displayScore = 0;


    int baseScore = resultIngredients.size() * 100;
    int comboBonus = session.maxCombo * 50;

    int finalScore = baseScore + noMissBonus + comboBonus + feverBonus;
    GameDifficulty diff = session.GetDifficulty();

    bool isNewRecord = OdenHighScoreData::Instance().TryUpdateHighScore(diff, finalScore);

    if (isNewRecord)
    {
        Logger::Log(U8("ハイスコア更新！"));
    }
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
#if 0
            if (resultIngredients[spawnIndex]->GetIsFeverModeIngredient())
            {
                addScore *= 2; // フィーバー中はスコア2倍
            }
#endif // 0

            AddScore(addScore);

            spawnTimer = 0.0f;
            ++spawnIndex;
            nextSpawnDelay = CalcSpawnDelay();
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

// spawnIndexに応じてスポーンタイムを変更する
float OdenResultScoreActor::CalcSpawnDelay() const
{
    int total = static_cast<int>(resultIngredients.size());

    // 最初の2個
    if (spawnIndex < 1)
        return 0.55f;

    // 最初の2個
    if (spawnIndex < 2)
        return 0.45f;

    // ラスト1個：溜め
    if (spawnIndex == total - 1)
        return 0.8f;   // ← ここが「溜め」

    // ラスト2個目：少しゆっくり
    if (spawnIndex == total - 2)
        return 0.6f;

    // ラスト3個目：少しゆっくり
    if (spawnIndex == total - 3)
        return 0.5f;



    // 中盤はだんだん早く
    float t = static_cast<float>(spawnIndex - 2) / (total - 4);
    float time = std::lerp<float>(0.3f, 0.1f, t); // 徐々に速く
    return time;

}
