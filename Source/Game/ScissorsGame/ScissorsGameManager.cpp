#include "pch.h"
#include "ScissorsGameManager.h"

#include "Engine/Scene/Scene.h"

void ScissorsGameManager::Initialize(const Transform& transform)
{
    // タイマーやスコアをリセットする
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
    totalScore = 0;
    combo = 0;
    maxTime = 45.0f;    // ここで制限時間を設定
    remainingTime = maxTime;
    satisfaction = 0.0f;
    isGameEnded = false;

    Logger::Log(U8("ゲームステートをリセットしました。"));
}

// スコアを加算する
void ScissorsGameManager::AddScore(float score)
{
    float addScore = score;
    totalScore += addScore;
    Logger::Log(U8("今の総スコア") + std::to_string(totalScore));
}



void ScissorsGameManager::EndGame()
{
    isGameEnded = true;

    // finish の演出を入れる
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIEndActor"))
    {
        //if (auto endUI = std::dynamic_pointer_cast<OdenUIEndActor>(actor))
        //{
        //    endUI->Play();
        //}
        //else
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
