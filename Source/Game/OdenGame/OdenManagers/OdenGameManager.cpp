#include "pch.h"
#include "OdenGameManager.h"

#include "Game/OdenGame/OdenGameSession.h"
#include "UI/Game/SceneTransitionManager.h"


void OdenGameManager::Initialize(const Transform& transform)
{
    // タイマーやスコアをリセットする
    Reset();

    // セッションもリセット
    OdenGameSession::Instance().Reset();
}

void OdenGameManager::Update(float deltaTime)
{
    if (isGameEnded)
        return;
    remainingTime -= deltaTime;
    remainingTime = std::max<float>(remainingTime, 0.0f);
    if (IsTimeUp())
    {
        EndGame();
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "ResultScene"), std::make_pair("type", types[rand() % 2]) });
    }
}

// ゲームのステートをリセットする
void OdenGameManager::Reset()
{
    totalScore = 0;
    combo = 0;
    maxTime = 15000.0f;    // ここで制限時間を設定
    remainingTime = maxTime;
    satisfaction = 0.0f;
    isGameEnded = false;
    //submitLogs.clear();
    //ingredientCount = {};
    Logger::Log(U8("ゲームステートをリセットしました。"));
}

// スコアを加算する
void OdenGameManager::AddScore(float score)
{
    //if (IsTimeUp()) return;
    totalScore += score;
    Logger::Log(U8("今の総スコア") + std::to_string(totalScore));
    //OdenGameSession::Instance().totalScore = totalScore;
}


// 提出ログを追加
void OdenGameManager::AddSubmitLog(EOdenType type, float score)
{
    //submitLogs.emplace_back(type, 1, 0.0f );

    //const size_t index = static_cast<size_t>(type);
    //ingredientCount[index]++;

    //auto& session = OdenGameSession::Instance();
    //session.submitLogs.push_back({ type,1,score });
    //session.ingredientCount[type]++;
}


void OdenGameManager::EndGame()
{
    isGameEnded = true;
    //auto& session = OdenGameSession::Instance();
    //session.totalScore = totalScore;
    //session.satisfaction = satisfaction;
    //session.ingredientCount = ingredientCount;
    //session.submitLogs = submitLogs;
}

