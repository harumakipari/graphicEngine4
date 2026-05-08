#include "pch.h"
#include "ScissorsGameManager.h"

#include "ScissorsUiEndActor.h"
#include "Engine/Scene/Scene.h"

void ScissorsGameManager::Initialize(const Transform& transform)
{
    // タイマーをリセットする
    Reset();
}

void ScissorsGameManager::Update(float deltaTime)
{
    if (isGameEnded || !isGameRunning)
        return;



    remainingTime -= deltaTime;
    remainingTime = std::max<float>(remainingTime, 0.0f);
    if (IsTimeUp())
    {
        //Logger::Log(U8("ゲームが終わりました"));
        //EndGame();
    }
}

// ゲームのステートをリセットする
void ScissorsGameManager::Reset()
{
    maxTime = 45.0f;    // ここで制限時間を設定
    remainingTime = maxTime;
    isGameEnded = false;

    Logger::Log(U8("ゲームステートをリセットしました。"));
}


void ScissorsGameManager::EndGame()
{
    isGameEnded = true;

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

// ゲーム開始処理
void ScissorsGameManager::StartGame()
{
    isGameRunning = true;
}
