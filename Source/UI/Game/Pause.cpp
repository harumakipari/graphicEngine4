#include "pch.h"
#include "Pause.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Utility/Time.h"

void Pause::Initialize(const Transform& transform)
{

    auto scene = GetOwnerScene();

    pausePanel = std::make_shared<UIImageComponent>("./Data/Textures/UI/pause_panel.png", "pause_panel");
    pausePanel->SetWorldPosition({ 967, 490 });
    pausePanel->SetPivot({ 0.5f,0.5f });
    pausePanel->SetScale({ 1.2f,1.2f });
    pausePanel->SetSize({ 741, 483 });
    pausePanel->SetVisible(false);
    scene->GetUIManager()->Add(pausePanel);

    menuButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/menu.png", "menu");
    menuButton->SetWorldPosition({ 100, 85 });
    menuButton->SetPivot({ 0.5f,0.5f });
    menuButton->SetSize({ 140, 140 });
    scene->GetUIManager()->Add(menuButton);
    //
    menuButton->onClick = [&]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
            pausePanel->SetVisible(true);
            pausePanel->SetEnable(true);

            closeButton->SetVisible(true);
            closeButton->SetEnable(true);

            returnTitleButton->SetVisible(true);
            returnTitleButton->SetEnable(true);

            menuButton->SetEnable(false);
            menuButton->SetVisible(false);

            Time::timeScale = 0.0f;

            GetOwnerScene()->SetPaused(true);
        };


    closeButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/close.png", "close");
    closeButton->SetWorldPosition({ 1281, 492 });
    closeButton->SetPivot({ 0.5f,0.5f });
    closeButton->SetSize({ 140, 140 });
    closeButton->SetVisible(false);
    closeButton->onClick = [&]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
            pausePanel->SetVisible(false);
            pausePanel->SetEnable(false);
            closeButton->SetEnable(false);
            closeButton->SetVisible(false);
            returnTitleButton->SetEnable(false);
            returnTitleButton->SetVisible(false);

            menuButton->SetEnable(true);
            menuButton->SetVisible(true);

            Time::timeScale = 1.0f;

            GetOwnerScene()->SetPaused(false);

        };
    GetOwnerScene()->GetUIManager()->Add(closeButton);

    returnTitleButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/exit.png", "exit");
    returnTitleButton->SetWorldPosition({ 977, 638 });
    returnTitleButton->SetPivot({ 0.5f,0.5f });
    returnTitleButton->SetSize({ 472, 183 });
    returnTitleButton->SetVisible(false);
    returnTitleButton->onClick = [&]()
        {

            CoreAudio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
            Time::timeScale = 1.0f;

            const char* types[] = { "0", "1" };
            Scene::_transition("LoadingScene", { std::make_pair("preload", "PuddingGameScene"), std::make_pair("type", types[rand() % 2]) });

        };


    GetOwnerScene()->GetUIManager()->Add(returnTitleButton);
}


