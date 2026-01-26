#include "pch.h"
#include "OdenGameManager.h"

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenGameSession.h"
#include "Game/OdenGame/OdenUIEndActor.h"
#include "UI/Game/SceneTransitionManager.h"


void OdenGameManager::Initialize(const Transform& transform)
{
    // イージングコンポーネントを作成
    easingBgm = std::make_shared<EasingRunner>();

    // タイマーやスコアをリセットする
    Reset();

    // セッションもリセット
    OdenGameSession::Instance().Reset();
}

void OdenGameManager::Update(float deltaTime)
{
    easingBgm->Tick(deltaTime);

    auto bgm = bgmAudio.lock();
    if (bgm)
    {
        bgm->SetPitch(bgmPitch);
    }


    if (isGameEnded || !isGameRunning)
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
    }
}

// ゲームのステートをリセットする
void OdenGameManager::Reset()
{
    totalScore = 0;
    combo = 0;
    maxTime =50.0f;    // ここで制限時間を設定
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
    //float addScore = CalcScore(score);
    float addScore = score;
    totalScore += addScore;
    Logger::Log(U8("今の総スコア") + std::to_string(totalScore));
    //OdenGameSession::Instance().totalScore = totalScore;
}

// 提出成功時の処理
void OdenGameManager::OnSubmitSuccess()
{
    AddCombo();

    if (feverState == EFeverState::Fever)
    {// フィーバー中に提出した具材を数える
        OdenGameSession::Instance().feverSubmitCount++;
    }

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

// 提出失敗時の処理
void OdenGameManager::OnSubmitMiss()
{
    Logger::Log(U8("提出ミスしている"));
    combo = 0;
    OdenGameSession::Instance().missCount++;
}

// フィーバー開始
void OdenGameManager::StartFeverMode()
{
    feverState = EFeverState::Fever;
    feverRemainingTime = feverTime;
    feverGauge = feverGaugeMax;

    // fever入ると＋3秒
    remainingTime += 3.0f;
    remainingTime = std::min<float>(remainingTime, maxTime);

    justFeverMode = true;
    justAppearWord = true;

    Logger::Log(U8("feverの時に＋３秒された")+std::to_string(remainingTime));

    // ピッチ の easing
    {
        TestEasingHandler handler;

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            1.2f,
            0.2f
        );

        handler.SetCompletedFunction([this]()
            {
                auto bgm = bgmAudio.lock();
                if (bgm)
                {
                    bgm->SetPitch(1.2f);
                }

            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return bgmPitch; };
        accessor.setter = [this](float t)
            {
                bgmPitch = t;
            };

        easingBgm->StartHandler(handler, accessor);
    }


    Logger::Log(U8("フィーバーモード突入！"));
}

// フィーバー終了
void OdenGameManager::EndFever()
{
    feverState = EFeverState::Charging;
    feverGauge = 0.0f;

    // ピッチ の easing
    {
        TestEasingHandler handler;

        handler.AddEasing(
            TestEaseType::OutExp,
            1.2f,
            1.0f,
            0.2f
        );

        handler.SetCompletedFunction([this]()
            {
                auto bgm = bgmAudio.lock();
                if (bgm)
                {
                    bgm->SetPitch(1.0f);
                }

            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return bgmPitch; };
        accessor.setter = [this](float t)
            {
                bgmPitch = t;
            };

        easingBgm->StartHandler(handler, accessor);
    }


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

// フィーバーに入った瞬間を取得する
bool OdenGameManager::ConsumeFeverMode()
{
    if (justFeverMode)
    {
        justFeverMode = false;
        return true;
    }
    return false;
}

// フィーバーに入った瞬間を取得する
bool OdenGameManager::ConsumeFeverWordAppear()
{
    if (justAppearWord)
    {
        justAppearWord = false;
        return true;
    }
    return false;
}

// コンボを加算する
void OdenGameManager::AddCombo()
{
    combo++;
    Logger::Log(U8("コンボ数") + std::to_string(combo));
    OdenGameSession::Instance().maxCombo = std::max<int>(OdenGameSession::Instance().maxCombo, combo);
}

void OdenGameManager::EndGame()
{
    isGameEnded = true;

    // finish の演出を入れる
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenUIEndActor"))
    {
        if (auto endUI = std::dynamic_pointer_cast<OdenUIEndActor>(actor))
        {
            endUI->Play();
        }
        else
        {
            const char* types[] = { "0", "1" };
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "ResultScene"), std::make_pair("type", types[rand() % 2]) });
        }
    }
    else
    {
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "ResultScene"), std::make_pair("type", types[rand() % 2]) });
    }


}

// ゲーム開始処理
void OdenGameManager::StartGame()
{
    isGameRunning = true;
}
