#include "pch.h"
#include "TitleBookActor.h"

#include "ScoreHistoryManager.h"
#include "UI/Game/SceneTransitionManager.h"
#include "TitleScene.h"

void TitleBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);

    CreateBookModel("./Data/TeamModels/Title/BookRightTitle.gltf", "./Data/TeamModels/Title/BookMiddleTitle.gltf");

    SetInitPageState(BookPageState::Closed);

    // ハイスコアの数字を乗せるページの親　左側
    std::string middleName = rightPage.parentName;

    // ステージ１　親を生成する
    {
        std::string scoreParentName = "high_first_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, middleName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.5f,-0.1f,-2.0f });

        firstStageHighScoreDisplay.Initialize(
            this,
            scoreParentName,
            "first_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, false);
    }

    // ステージ2　親を生成する
    {
        std::string scoreParentName = "high_bobbin_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, middleName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.5f,-0.1f,1.1f });

        bobbinStageHighScoreDisplay.Initialize(
            this,
            scoreParentName,
            "bobbin_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, false);
    }

    // 裏表紙のページ
    // ステージ3　親を生成する
    {
        std::string scoreParentName = "high_redirect_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-1.5f });

        redirectStageHighScoreDisplay.Initialize(
            this,
            scoreParentName,
            "redirect_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    // ステージ4　親を生成する
    {
        std::string scoreParentName = "high_difficult_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -3.2f,0.0f,0.1f });

        difficultStageHighScoreDisplay.Initialize(
            this,
            scoreParentName,
            "difficult_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    // ボスステージハイスコア
    {
        std::string scoreParentName = "high_boss_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,1.7f });

        bossStageHighScoreDisplay.Initialize(
            this,
            scoreParentName,
            "boss_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    // 矢印ボタンのUIを作成する
    CreateButtonArrow();

    // AボタンのUIを生成する
    auto uiManager = GetOwnerScene()->GetUIManager();
    controlAButton = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/A.png", "A");
    controlAButton->SetWorldPosition({ 880, 920 });
    controlAButton->SetSize({ 80, 80 });
    controlAButton->SetPivot({ 0.5f,0.5f });
    controlAButton->SetVisible(false);
    uiManager->Add(controlAButton);
}

void TitleBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);

    if (bookState == BookPageState::Closed)
    {// 本が閉じている時に
        if (InputSystem::IsGamepadConnected())
        {// ゲームパッドが繋がれていたら
            controlAButton->SetVisible(true);
        }
        else
        {
            controlAButton->SetVisible(false);
        }
    }
    else
    {
        controlAButton->SetVisible(false);
    }

    // ハイスコアを取得する
    // ステージ１
    int firstHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::FIRST);
    firstStageHighScoreDisplay.SetValue(firstHighScore);

    // ステージ2
    int bobbinHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOBBIN_FIRST);
    bobbinStageHighScoreDisplay.SetValue(bobbinHighScore);

    // 裏表紙
    // ステージ3
    int redirectHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::REFLECT_WALL);
    redirectStageHighScoreDisplay.SetValue(redirectHighScore);

    // ステージ4
    int difficultHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::DIFFICULT);
    difficultStageHighScoreDisplay.SetValue(difficultHighScore);

    // ボス戦
    int bossHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOSS);
    bossStageHighScoreDisplay.SetValue(bossHighScore);

}

void TitleBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

// コントローラー対応の本が開く処理
void TitleBookActor::HandlePadInput()
{
    bool pushA = InputSystem::GetInputState("GamePadA", InputStateMask::Trigger);

    bool pushR = InputSystem::GetInputState("BookRight", InputStateMask::Trigger);
    bool pushL = InputSystem::GetInputState("BookLeft", InputStateMask::Trigger);

    switch (bookState)
    {
    case BookPageState::Closed:
        if (pushA)
        {
            auto scene = GetOwnerScene();
            if (auto titleScene = dynamic_cast<TitleScene*>(scene))
            {
                titleScene->StartToSelect();
            }
        }
        break;
    case BookPageState::FirstPage:
        if (pushL)
        {
            auto scene = GetOwnerScene();
            if (auto titleScene = dynamic_cast<TitleScene*>(scene))
            {
                titleScene->StartToTitle();
            }
        }
        if (pushR)
        {
            // 二ページ目を開く
            OpenSecondPage(2.0f);
        }
        break;
    case BookPageState::SecondPage:
        if (pushL)
        {
            // 一ページ目に戻る
            CloseSecondPage(2.0f);
        }
#if 0   // Aボタン表示のバグあり
        if (pushR)
        {
            // タイトルシーンへ戻る
            auto scene = GetOwnerScene();
            if (auto titleScene = dynamic_cast<TitleScene*>(scene))
            {
                titleScene->StartToTitle();
            }
        }
#endif
        break;
    }
}

// 矢印ボタンのUIを作成する
void TitleBookActor::CreateButtonArrow()
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    DirectX::XMFLOAT2 uiLeftPos = { 12, 875 };
    DirectX::XMFLOAT2 uiRightPos = { 1650, 875 };
    DirectX::XMFLOAT2 uiArrowSize = { 250, 150 };

    // 一ページ左
    firstButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow.png", "title_arrow");
    firstButtons.left->SetWorldPosition(uiLeftPos);
    firstButtons.left->SetSize(uiArrowSize);
    uiManager->Add(firstButtons.left);

    firstButtons.left->onClick = [this]()
        {
            // 本を閉じる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            auto scene = GetOwnerScene();
            if (auto titleScene = dynamic_cast<TitleScene*>(scene))
            {
                titleScene->StartToTitle();
            }
        };

    // ゲームパッドの画像を設定する
    firstButtons.gamePadLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow.png");

    // 一ページ右
    firstButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/ranking_arrow.png", "ranking_arrow");
    firstButtons.right->SetWorldPosition(uiRightPos);
    firstButtons.right->SetSize(uiArrowSize);
    uiManager->Add(firstButtons.right);

    firstButtons.right->onClick = [this]()
        {
            OpenSecondPage(2.0f);
            // ページをめくる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
        };
    // ゲームパッドの画像を設定する
    firstButtons.gamePadRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/ranking_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/ranking_arrow.png");

    // 二ページ目左
    secondButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/stage_select_arrow.png", "stage_select_arrow");
    secondButtons.left->SetWorldPosition(uiLeftPos);
    secondButtons.left->SetSize(uiArrowSize);
    uiManager->Add(secondButtons.left);

    secondButtons.left->onClick = [this]()
        {
            // 一ページ目に戻る
            CloseSecondPage(2.0f);
            // ページをめくる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
        };
    // ゲームパッドの画像を設定する
    secondButtons.gamePadLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/stage_select_arrow_control.png");
    // キーボードの画像を設定する
    secondButtons.keyboardLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/stage_select_arrow.png");



#if 0   // Aボタン表示のバグあり
    // 二ページ目右
    secondButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow_right.png", "title_arrow_right");
    secondButtons.right->SetWorldPosition({ 1000, 800 });
    secondButtons.right->SetSize({ 400, 150 });
    uiManager->Add(secondButtons.right);

    secondButtons.right->onClick = [this]()
        {
            // 本を閉じる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            auto scene = GetOwnerScene();
            if (auto titleScene = dynamic_cast<TitleScene*>(scene))
            {
                titleScene->StartToTitle();
            }
        };

#endif // 0
}
