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

    if (feverState == EFeverState::Fever)
    {
        feverRemainingTime -= deltaTime;
        feverGauge = feverGaugeMax * (feverRemainingTime / feverTime);

        if (feverRemainingTime <= 0.0f)
        {
            EndFever();
        }
    }

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
    maxTime = 50.0f;    // ここで制限時間を設定
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
    float addScore = CalcScore(score);
    totalScore += addScore;
    Logger::Log(U8("今の総スコア") + std::to_string(totalScore));
    //OdenGameSession::Instance().totalScore = totalScore;
}

// 提出成功時の処理
void OdenGameManager::OnSubmitSuccess()
{
    AddCombo();

    if (feverState == EFeverState::Charging)
    {
        // コンボに応じてフィーバーゲージを加算
        float addFeverGauge = feverGaugeMax / static_cast<float>(feverTriggerCombo);

        feverGauge += addFeverGauge;

        if (feverGauge >= feverGaugeMax)
        {
            StartFeverMode();
        }
    }
}

// フィーバー開始
void OdenGameManager::StartFeverMode()
{
    feverState = EFeverState::Fever;
    feverRemainingTime = feverTime;
    feverGauge = feverGaugeMax;

    Logger::Log(U8("フィーバーモード突入！"));
}

// フィーバー終了
void OdenGameManager::EndFever()
{
    feverState = EFeverState::Charging;
    feverGauge = 0.0f;
    ResetCombo();

    Logger::Log(U8("フィーバー終了"));
}

// スコアを二倍にする
float OdenGameManager::CalcScore(float baseScore) const
{
    if (feverState == EFeverState::Fever)
    {
        Logger::Log(U8("フィーバー中のためにスコアを二倍にする"));
        return baseScore * 2.0f;
    }
    return baseScore;
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

