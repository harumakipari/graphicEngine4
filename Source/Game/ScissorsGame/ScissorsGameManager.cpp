#include "pch.h"
#include "ScissorsGameManager.h"

#include "ScissorsUiEndActor.h"
#include "ScoreCalculator.h"
#include "Engine/Scene/Scene.h"
#include "UI/Game/Pause.h"

void ScissorsGameManager::Initialize(const Transform& transform)
{
    // タイマーをリセットする
    Reset();
}

void ScissorsGameManager::Update(float deltaTime)
{
    if (isGameEnded || !isGameRunning)
        return;

    gameTimer += deltaTime;
}

// ゲームのステートをリセットする
void ScissorsGameManager::Reset()
{
    gameTimer = 0.0f;
    isGameEnded = false;

    Logger::Log(U8("ゲームステートをリセットしました。"));
}


void ScissorsGameManager::EndGame(bool playerDead)
{
    isGameEnded = true;

    if (auto pause=GetOwnerScene()->GetActorManager()->GetActorOfType<Pause>())
    {
        pause->HidePauseMenu();
    }

    // 入力を無効化する
    gameInputEnabled = false;
    Logger::Log(U8("入力を無効化する"));

    // 所要時間を記録する
    ScoreSystem::RecordGameTime(gameTimer);


}

// 終了演出を開始する
void ScissorsGameManager::StartFinishPerform()
{
    {// playerが死亡していなかったら、
        // finish の演出を入れる
        if (auto actor = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsUiEndActor>())
        {
            // クリア演出
            actor->Play();
        }
        else
        {
#if 1
            const char* types[] = { "0", "1" };
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("type", types[rand() % 2]) });
#else
            const char* types[] = { "0", "1" };
            Scene::_transition("LoadingScene", { std::make_pair("preload", "ResultScene"),{"difficulty", "0"} });
#endif // 0
        }
    }
}

// ゲーム開始処理
void ScissorsGameManager::StartGame()
{
    // 入力を有効化する
    gameInputEnabled = true;
}


// ゲーム測定開始処理
void ScissorsGameManager::StartTimer()
{
    isGameRunning = true;
}