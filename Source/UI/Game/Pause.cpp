#include "pch.h"
#include "Pause.h"

#include "SceneTransitionManager.h"
#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Utility/Time.h"

void Pause::Initialize(const Transform& transform)
{
    const auto scene = GetOwnerScene();

    pausePanel = std::make_shared<UIImageComponent>("./Data/Textures/UI/pause_panel.png", "pause_panel");
    pausePanel->SetWorldPosition({ 967, 490 });
    pausePanel->SetPivot({ 0.5f,0.5f });
    pausePanel->SetScale({ 1.2f,1.2f });
    pausePanel->SetSize({ 741, 483 });
    pausePanel->SetVisible(false);
    pausePanel->zOrder = 100; // 手前に描画する
    scene->GetUIManager()->Add(pausePanel);

    // メニューボタン
    {
        menuButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/menu.png", "menu");
        menuButton->SetWorldPosition({ 100, 85 });
        menuButton->SetPivot({ 0.5f,0.5f });
        menuButton->SetSize({ 140, 140 });
        menuButton->zOrder = 100; // 手前に描画する
        scene->GetUIManager()->Add(menuButton);
        menuButton->onClick = [&]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/escape_se.wav");
            pausePanel->SetVisible(true);
            pausePanel->SetEnable(true);

            closeButton->SetVisible(true);
            closeButton->SetEnable(true);

            returnTitleButton->SetVisible(true);
            returnTitleButton->SetEnable(true);

            retryButton->SetVisible(true);
            retryButton->SetEnable(true);

            menuButton->SetEnable(false);
            menuButton->SetVisible(false);


            Time::timeScale = 0.0f;

            GetOwnerScene()->SetPaused(true);
        };
    }


    closeButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/close.png", "close");
    closeButton->SetWorldPosition({ 1281, 492 });
    closeButton->SetPivot({ 0.5f,0.5f });
    closeButton->SetSize({ 140, 140 });
    closeButton->SetVisible(false);
    closeButton->SetEnable(false);
    closeButton->zOrder = 105; // 手前に描画する

    closeButton->onClick = [&]()
        {
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            pausePanel->SetVisible(false);
            pausePanel->SetEnable(false);
            closeButton->SetEnable(false);
            closeButton->SetVisible(false);
            returnTitleButton->SetEnable(false);
            returnTitleButton->SetVisible(false);
            retryButton->SetEnable(false);
            retryButton->SetVisible(false);
            state = PauseState::ResumeCountdown;
            countdownTime = 3.0f;
        };
    GetOwnerScene()->GetUIManager()->Add(closeButton);

    returnTitleButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/back_to_title.png", "back_to_title");
    returnTitleButton->SetWorldPosition({ 977, 638 });
    returnTitleButton->SetPivot({ 0.5f,0.5f });
    returnTitleButton->SetSize({ 472, 183 });
    returnTitleButton->SetVisible(false);
    returnTitleButton->SetEnable(false);
    returnTitleButton->zOrder = 105; // 手前に描画する
    returnTitleButton->onClick = [&]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            Time::timeScale = 1.0f;

            const char* types[] = { "0", "1" };
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("type", types[rand() % 2]) });
        };

    GetOwnerScene()->GetUIManager()->Add(returnTitleButton);


    retryButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/retry.png", "retry");
    retryButton->SetWorldPosition({ 977, 838 });
    retryButton->SetPivot({ 0.5f,0.5f });
    retryButton->SetSize({ 472, 183 });
    retryButton->SetVisible(false);
    retryButton->SetEnable(false);
    retryButton->zOrder = 105; // 手前に描画する
    retryButton->onClick = [&]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            Time::timeScale = 1.0f;

            const char* types[] = { "0", "1" };
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
        };

    GetOwnerScene()->GetUIManager()->Add(retryButton);


    for (int i = 0; i < 3; i++)
    {
        const int num = 3 - i; // 3,2,1
        std::string filename = "./Data/Textures/UI/CountDown_" + std::to_string(num) + ".png";

        countDownImages[i] = std::make_shared<UIImageComponent>(filename, "countDown_" + std::to_string(num));

        countDownImages[i]->SetWorldPosition({ 967, 490 });
        countDownImages[i]->SetPivot({ 0.5f,0.5f });
        countDownImages[i]->SetScale({ 4.5f,4.5f });
        countDownImages[i]->SetSize({ 150, 200 });
        countDownImages[i]->SetVisible(false);
        countDownImages[i]->zOrder = 110; // 手前に描画する

        scene->GetUIManager()->Add(countDownImages[i]);
    }
    //pausePanel = std::make_shared<UIImageComponent>("./Data/Textures/UI/CountDown_1.png", "pause_panel");
    //pausePanel->SetVisible(false);
    //scene->GetUIManager()->Add(pausePanel);

}

void Pause::Update(float deltaTime)
{
    if (state != PauseState::ResumeCountdown)
        return;

    countdownTime -= Time::UnscaledDeltaTime();

    const int current = static_cast<int>(ceil(countdownTime)); // 3,2,1

    // 全部消す
    for (auto& img : countDownImages)
    {
        img->SetVisible(false);
        img->SetEnable(false);
    }

    if (current >= 1 && current <= 3)
    {
        int index = 3 - current; // 3→0, 2→1, 1→2
        countDownImages[index]->SetVisible(true);
        countDownImages[index]->SetEnable(true);

        float t = countdownTime - floor(countdownTime);
        float scale = std::lerp(6.0f, 4.5f, t);
        countDownImages[index]->SetScale({ scale, scale });

        float alpha = std::lerp(0.0f, 1.0f, t);
        countDownImages[index]->SetColor({ 1,1,1, alpha });

        if (current != lastCountdownNumber)
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/game_countDown_se.wav",3.0f);
            lastCountdownNumber = current;
        }
    }


    if (countdownTime <= 0.0f)
    {
        // 全部消す
        for (auto& img : countDownImages)
        {
            img->SetVisible(false);
            img->SetEnable(false);
        }

        Time::timeScale = 1.0f;
        GetOwnerScene()->SetPaused(false);
        state = PauseState::Playing;
        lastCountdownNumber = -1;
        menuButton->SetEnable(true);
        menuButton->SetVisible(true);

    }
}


