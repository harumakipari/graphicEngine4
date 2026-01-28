#include "pch.h"
#include "OdenResultScoreActor.h"
#include <magic_enum.hpp>

#include "OdenManagers/OdenGameManager.h"
#include "OdenGameSession.h"
#include "OdenHighScoreData.h"
#include "OdenRank.h"
#include "OdenResultSkewerActor.h"
#include "Engine/Scene/Scene.h"
#include "UI/FontManager.h"

static std::vector<OdenSubmitLog> CreateDebugSubmitLogs()
{
    return {
        { EOdenType::Chikuwa,    1, 300.0f },
        { EOdenType::Donut,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },

        { EOdenType::Cake,   1, 100.0f },
        { EOdenType::Goboten,   1, 100.0f },
        { EOdenType::Donut,   1, 100.0f },

        { EOdenType::Daikon,   1, 100.0f },
        { EOdenType::Hanpen,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },

        { EOdenType::Donut,   1, 100.0f },
        { EOdenType::Konnyaku,   1, 100.0f },
        { EOdenType::Goboten,   1, 100.0f },

        { EOdenType::Tsukune,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },
        { EOdenType::Shirataki,   1, 100.0f },

        { EOdenType::Konnyaku,   1, 100.0f },
        { EOdenType::Daikon,     1, 400.0f },
        { EOdenType::Cake,   1, 100.0f },

        { EOdenType::Konnyaku,   1, 100.0f },
        { EOdenType::Daikon,     1, 400.0f },
        { EOdenType::Donut,   1, 100.0f },

        { EOdenType::Cake,   1, 100.0f },
        { EOdenType::Tsukune,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },

        { EOdenType::Kobumusubi,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },
        { EOdenType::Shirataki,   1, 100.0f },


        { EOdenType::Goboten,   1, 100.0f },
            { EOdenType::Tsukune,   1, 100.0f },
        { EOdenType::Egg,     1, 200.0f },
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
            Logger::Log("  " + std::string(magic_enum::enum_name(static_cast<EOdenType>(i))) + " : " + std::to_string(count));
        }
    }
    // ノーミスのスコア
    int noMissBonus = (session.missCount == 0) ? OdenGameSession::GetOdenNoMissBonus() : 0;
    // フィーバー中の提供のスコアの計算
    int feverBonus = session.feverSubmitCount * OdenGameSession::GetOdenScoreByOnce();  // フィーバーは二倍になるから
    int maxCombo = session.maxCombo;
    Logger::Log(U8("noMissBonus") + std::to_string(noMissBonus));
    Logger::Log(U8("feverBonus") + std::to_string(feverBonus));
    Logger::Log(U8("連続成功人数") + std::to_string(maxCombo));

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
    int skewerIndex = 0;
    std::shared_ptr<OdenResultSkewerActor> currentSkewer;
    int ingredientInSkewer = 0;

    for (auto& log : *logs)
    {
        for (int i = 0; i < log.count; ++i)
        {
#if 1
            std::string ingredientName = std::string(magic_enum::enum_name(log.type));
            if (ingredientName.empty())
            {
                Logger::Log("Warning: SubmitLog has invalid EOdenType");
                continue; // 作らない
            }
            // ここで配置位置を決めて生成する

            float x = -20.1f + globalIndex * 3.0f;
            float y = 10.723f + globalIndex * 1.0f;
            float z = -5.506f;
            Logger::Log(U8("グローバルインデックス") + std::to_string(globalIndex));
            //float x =  globalIndex * 0.5f; // 横に並べる      
            //float y = 10.723f+globalIndex * 1.0f;                      // 高さ固定
            //float z = -5.506f + globalIndex * 1.0f;
            Transform ingredientTr(DirectX::XMFLOAT3{ x,y,z }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 2.5f,2.5f,2.5f });
            auto ingredientActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultIngredientActor>("OdenResultIngredient", ingredientTr, ingredientName);
            ingredientActor->SetFeverModeIngredient(log.wasFever);// フィーバー中に提出された具材かどうかを設定する
            resultIngredients.push_back(ingredientActor);
            globalIndex++;
#else
            if (ingredientInSkewer == 0)
            {// 三個に一個串を作成する
                //float x = 1.2f + skewerIndex * 5.0f;
                float x = -20.1f + skewerIndex * 5.0f;
                //float y = 6.723f;
                float y = 0.1f;
                float z = -5.506f;

                Transform skewerTr(DirectX::XMFLOAT3{ x, y, z }, DirectX::XMFLOAT4{ 0,0,0,1 }, DirectX::XMFLOAT3{ 1,1,1 });
                currentSkewer = GetOwnerScene()->GetActorManager()
                    ->CreateAndRegisterActorWithTransform<OdenResultSkewerActor>("OdenSkewer", skewerTr);

                skewerIndex++;
            }

            std::string ingredientName = std::string(magic_enum::enum_name(log.type));

            auto ingredientActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultIngredientActor>("OdenResultIngredient", Transform{}, ingredientName);

            ingredientActor->SetFeverModeIngredient(log.wasFever);

            // 串に追加
            currentSkewer->AddIngredient(ingredientActor, ingredientInSkewer);

            resultIngredients.push_back(ingredientActor);

            ingredientInSkewer++;
            if (ingredientInSkewer >= 3)
                ingredientInSkewer = 0;
#endif // 0
        }

    }
    displayScore = 0;

    // 基本スコアの計算
    int baseScore = static_cast<int>(resultIngredients.size()) * OdenGameSession::GetOdenScoreByOnce();
    Logger::Log(U8("基本スコア") + std::to_string(baseScore));
    // 連続正解のスコアの計算
    int comboBonus = session.maxCombo * OdenGameSession::GetOdenComboBonus(); // 連続正解　＊　
    Logger::Log(U8("連続正解スコア") + std::to_string(comboBonus));
    // 最終的なスコアの計算
    int finalScore = baseScore + noMissBonus + comboBonus + feverBonus;
    Logger::Log(U8("最終的なスコア") + std::to_string(finalScore));
    // クリアした難易度を取得する
    GameDifficulty diff = session.GetDifficulty();

    // ランクの計算をする
    auto rankResult = EvaluateRank(session.GetDifficulty(), finalScore);
    // 次のランクまで必要なスコア
    int needScore = rankResult.isMaxRank ? 0 : rankResult.nextRankScore - finalScore;

    Logger::Log(U8("今回のランク: ") + std::string(magic_enum::enum_name(rankResult.current)));
    Logger::Log(U8("次のランクまでに必要なスコア") + std::to_string(needScore));


    bool isNewRecord = OdenHighScoreData::Instance().TryUpdateHighScore(diff, finalScore);

    if (isNewRecord)
    {
        Logger::Log(U8("ハイスコア更新！"));
    }


    Logger::Log("=============================");

    auto uiManager = GetOwnerScene()->GetUIManager();
    XMFLOAT2 uiPos = { 500.0f,500.0f };
    XMFLOAT2 uiSize = { 400.0f,150.0f };

#if 0
    // 合計個数UI描画
    totalCountUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_total_count.png", "result_total_count");
    totalCountUi->SetWorldPosition({ uiPos.x, uiPos.y });
    totalCountUi->SetPivot({ 0.5f,0.5f });
    totalCountUi->SetSize(uiSize);
    uiManager->Add(totalCountUi);

    uiPos.y += 140.0f;

    // 注文ミスをしないUI描画
    noMissOrderUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_no_miss_order.png", "result_no_miss_order");
    noMissOrderUi->SetWorldPosition({ uiPos.x, uiPos.y });
    noMissOrderUi->SetPivot({ 0.5f,0.5f });
    noMissOrderUi->SetSize(uiSize);
    uiManager->Add(noMissOrderUi);

    uiPos.y += 140.0f;

    // 連続正解の個数UI描画
    streakCountUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_streak_count.png", "result_streak_count");
    streakCountUi->SetWorldPosition({ uiPos.x, uiPos.y });
    streakCountUi->SetPivot({ 0.5f,0.5f });
    streakCountUi->SetSize(uiSize);
    uiManager->Add(streakCountUi);

    uiPos.y += 140.0f;

    // フィーバー中の提供の個数UI描画
    feverCountUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_fever_count.png", "result_fever_count");
    feverCountUi->SetWorldPosition({ uiPos.x, uiPos.y });
    feverCountUi->SetPivot({ 0.5f,0.5f });
    feverCountUi->SetSize(uiSize);
    uiManager->Add(feverCountUi);

    // ランクS
    rankSUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_rank_s.png", "result_rank_s");
    rankSUi->SetWorldPosition({ uiPos.x, uiPos.y });
    rankSUi->SetPivot({ 0.5f,0.5f });
    rankSUi->SetSize(uiSize);
    uiManager->Add(rankSUi);

    std::shared_ptr<UIImageComponent> rankAUi; // ランクA
    std::shared_ptr<UIImageComponent> rankBUi; // ランクB
    std::shared_ptr<UIImageComponent> rankCUi; // ランクC
    std::shared_ptr<UIImageComponent> rankDUi; // ランクD

    // 次のランクSの時に表示するUI
    nextRankSUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/Result/result_rank_no_next.png", "result_rank_no_next");
    nextRankSUi->SetWorldPosition({ uiPos.x, uiPos.y });
    nextRankSUi->SetPivot({ 0.5f,0.5f });
    nextRankSUi->SetSize(uiSize);
    uiManager->Add(nextRankSUi);

    std::shared_ptr<UIImageComponent> nextRankAUi; // 次のランクAの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankBUi; // 次のランクBの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankCUi; // 次のランクCの時に表示するUI
    std::shared_ptr<UIImageComponent> nextRankDUi; // 次のランクDの時に表示するUI

#endif // 0



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


    // 食材の順番登場
    if (spawnIndex < resultIngredients.size())
    {
        spawnTimer += deltaTime;
        if (spawnTimer >= nextSpawnDelay)
        {
            resultIngredients[spawnIndex]->AppearIngredient();

            int addScore = OdenGameSession::GetOdenScoreByOnce(); 
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


    std::shared_ptr<UITextComponent> noMissScoreTextUi; // 注文ミスをしないスコア数字描画
    std::shared_ptr<UITextComponent> totalCountTextUi; // 合計個数の数字描画
    std::shared_ptr<UITextComponent> streakCountTextUi; // 連続正解の個数テキスト描画
    std::shared_ptr<UITextComponent> feverCountTextUi; // フィーバー中の提供の個数テキスト描画


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

    //return 0.5f;

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
