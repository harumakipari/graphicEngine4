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
        scoreRoot->SetRelativeLocationDirect({ -0.6f,-0.1f,-0.7f });
        scoreRoot->SetRelativeScaleDirect({ 0.77f,0.8f,0.8f });

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
        scoreRoot->SetRelativeLocationDirect({ -0.6f,-0.1f,1.0f });
        scoreRoot->SetRelativeScaleDirect({ 0.77f,0.8f,0.8f });

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
        redirectScorePosition = { -3.9f,0.0f,-1.3f };
        scoreRedirectRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRedirectRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRedirectRoot->SetRelativeLocationDirect(redirectScorePosition);
        scoreRedirectRoot->SetRelativeScaleDirect({ 0.77f,0.8f,0.8f });

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
        difficultScorePosition = { -3.9f,0.0f,0.1f };
        scoreDifficultRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreDifficultRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreDifficultRoot->SetRelativeLocationDirect(difficultScorePosition);
        scoreDifficultRoot->SetRelativeScaleDirect({ 0.77f,0.8f,0.8f });

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
        bossScorePosition = { -3.9f,0.0f,1.6f };
        scoreBossRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreBossRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreBossRoot->SetRelativeLocationDirect(bossScorePosition);
        scoreBossRoot->SetRelativeScaleDirect({ 0.77f,0.8f,0.8f });

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

    {
        // コントローラー対応用
        controlButtonOnImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/A.png");
        controlButtonOffImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/A_off.png");
        // キーボード対応用
        keyBoardButtonOnImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/mouseClick.png");
        keyBoardButtonOffImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/mouseClick_off.png");

        auto uiManager = GetOwnerScene()->GetUIManager();

        // Aボタン/マウスのUIを生成する
        controlAOnButton = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/mouseClick.png", "Tutorial_Mouse_Click");     // マウスのクリック
        controlAOnButton->SetWorldPosition(mousePos);
        controlAOnButton->SetSize(mouseSize);
        controlAOnButton->zOrder = 5;
        controlAOnButton->SetVisible(false);
        uiManager->Add(controlAOnButton);

        // Aボタン/マウスのUIを生成する
        controlAOffButton = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/mouseClick_off.png", "Tutorial_Mouse_Click_Off");     // マウスのクリックオフ
        controlAOffButton->SetWorldPosition(mousePos);
        controlAOffButton->SetSize(mouseSize);
        controlAOffButton->SetVisible(false);
        controlAOffButton->zOrder = 5;
        uiManager->Add(controlAOffButton);

        // 矢印のUIを生成する
        mouseArrowImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/arrow_left.png", "arrow_left");
        mouseArrowImage->SetWorldPosition({ 880, 920 });
        mouseArrowImage->SetSize({ 90, 75 });
        mouseArrowImage->SetPivot({ 0.5f,0.5f });
        mouseArrowImage->SetVisible(true);
        uiManager->Add(mouseArrowImage);

        rankImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/title_patch_title.png", "title_patch_title");
        rankImage->SetWorldPosition(patchSize);
        rankImage->SetVisible(true);
        rankImage->SetPivot({ 0.5f,0.5f });
        uiManager->Add(rankImage);

        selectImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/title_patch_select.png", "title_patch_result");
        selectImage->SetWorldPosition(patchSize);
        selectImage->SetVisible(true);
        selectImage->SetPivot({ 0.5f,0.5f });
        uiManager->Add(selectImage);
    }

    // タイトルシーンの本は最初は閉じている状態なので、ステージ選択のインデックスは0にしておく
    selectedStageIndex = 0;
}

void TitleBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
    elapsedTime += deltaTime;
#if 0
    if (bookState == BookPageState::Closed)
    {// 本が閉じている時に
        controlAOnButton->SetVisible(true);

        if (InputSystem::IsGamepadConnected())
        {// ゲームパッドが繋がれていたら
            controlAOnButton->SetTexture(controlButtonOnImage);
        }
        else
        {
            controlAOnButton->SetTexture(keyBoardButtonOnImage);
        }
    }
    else
    {
        controlAOnButton->SetVisible(false);
        controlAOffButton->SetVisible(false);
    }
#else
    if (bookState == BookPageState::Closed)
    {// 本が閉じている時に
        // マウスクリック点滅
        UpdateMouseClickBlink(deltaTime);
        // マウスクリックの表示を切り替える
        ShowMouseClick(true);

        if (InputSystem::IsGamepadConnected())
        {// ゲームパッドが繋がれていたら
            mouseArrowImage->SetVisible(false);
        }
        else
        {
            mouseArrowImage->SetVisible(true);
        }

    }
    else
    {
        // マウスクリックの表示を切り替える
        ShowMouseClick(false);
        // マウスクリックの点滅をリセットする
        ResetMouseClickBlink();

        mouseArrowImage->SetVisible(false);

    }

#endif // 0
    if (controlAOnButton)
    {
        controlAOnButton->SetWorldPosition(mousePos);
        controlAOnButton->SetSize(mouseSize);
    }

    if (mouseArrowImage)
    {
        arrowPos.y = arrowBasePos.y + 10.0f * std::sinf(elapsedTime * 3.0f);
        mouseArrowImage->SetWorldPosition(arrowPos);
        mouseArrowImage->SetSize(arrowSize);
        mouseArrowImage->SetWorldAngleDegree(arrowAngle);
    }

    // リザルトイメージ
    rankImage->SetWorldPosition(patchPos);
    rankImage->SetSize(patchSize);
    rankImage->SetWorldAngleDegree(patchAngle);

    // セレクトイメージ
    selectImage->SetWorldPosition(patchPos);
    selectImage->SetSize(patchSize);
    selectImage->SetWorldAngleDegree(patchAngle);


    // ハイスコアイメージ
    if (bookState == BookPageState::SecondPage)
    {
        rankImage->SetVisible(true);
    }
    else
    {
        rankImage->SetVisible(false);
    }

    // セレクトイメージ
    if (bookState == BookPageState::FirstPage)
    {
        selectImage->SetVisible(true);
    }
    else
    {
        selectImage->SetVisible(false);
    }



    // ハイスコアを取得する
    // ステージ１
    int firstHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::FIRST);
    firstStageHighScoreDisplay.SetValue(firstHighScore);
    firstStageHighScoreDisplay.SetColor(rankFirstColor);

    // ステージ2
    int bobbinHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOBBIN_FIRST);
    bobbinStageHighScoreDisplay.SetValue(bobbinHighScore);
    bobbinStageHighScoreDisplay.SetColor(rankBobbinColor);

    // 裏表紙
    // ステージ3
    int redirectHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::REFLECT_WALL);
    redirectStageHighScoreDisplay.SetValue(redirectHighScore);
    redirectStageHighScoreDisplay.SetColor(rankRedirectColor);

    // ステージ4
    int difficultHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::DIFFICULT);
    difficultStageHighScoreDisplay.SetValue(difficultHighScore);
    difficultStageHighScoreDisplay.SetColor(rankDifficultColor);

    // ボス戦
    int bossHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOSS);
    bossStageHighScoreDisplay.SetValue(bossHighScore);
    bossStageHighScoreDisplay.SetColor(rankBossColor);
#if 0
    float scoreOffsetY = std::lerp(-0.1f, 0.0f, bookTwoAlpha);
    scoreRedirectRoot->SetRelativeLocationDirect({ redirectScorePosition.x,scoreOffsetY, redirectScorePosition.z });
    scoreDifficultRoot->SetRelativeLocationDirect({ difficultScorePosition.x,scoreOffsetY, difficultScorePosition.z });
    scoreBossRoot->SetRelativeLocationDirect({ bossScorePosition.x,scoreOffsetY, bossScorePosition.z });
#endif // 0


}

void TitleBookActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    BookBaseActor::DrawImGuiDetails();
    ImGui::DragFloat2("arrowSize", &arrowSize.x);
    ImGui::DragFloat2("arrowPos", &arrowPos.x);
    ImGui::DragFloat("arrowAngle", &arrowAngle);

    ImGui::DragFloat2("mouseSize", &mouseSize.x);
    ImGui::DragFloat2("mousePos", &mousePos.x);

    ImGui::ColorEdit4("rankBossColor", &rankBossColor.x);
    ImGui::ColorEdit4("rankFirstColor", &rankFirstColor.x);
    ImGui::ColorEdit4("rankBobbinColor", &rankBobbinColor.x);
    ImGui::ColorEdit4("rankRedirectColor", &rankRedirectColor.x);
    ImGui::ColorEdit4("rankDifficultColor", &rankDifficultColor.x);

#endif

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


void TitleBookActor::UpdateMouseClickBlink(float deltaTime)
{
    if (InputSystem::IsGamepadConnected())
    {//　コントローラー対応
        controlAOnButton->SetTexture(controlButtonOnImage);
        controlAOffButton->SetTexture(controlButtonOffImage);
    }
    else
    {
        controlAOnButton->SetTexture(keyBoardButtonOnImage);
        controlAOffButton->SetTexture(keyBoardButtonOffImage);
    }

    if (isUpdateMouse)
    {
        mouseBlinkTimer += deltaTime;

        if (mouseBlinkTimer >= mouseBlinkInterval)
        {
            mouseBlinkTimer = 0.0f;
            isMouseClickOn = !isMouseClickOn;

            controlAOnButton->SetVisible(isMouseClickOn);
            controlAOffButton->SetVisible(!isMouseClickOn);
        }
    }
}

void TitleBookActor::ShowMouseClick(bool visible)
{
    controlAOnButton->SetVisible(visible && isMouseClickOn);
    controlAOffButton->SetVisible(visible && !isMouseClickOn);
    isUpdateMouse = true;
}

void TitleBookActor::ResetMouseClickBlink()
{
    mouseBlinkTimer = 0.0f;
    isMouseClickOn = false;
    controlAOnButton->SetVisible(false);
    controlAOffButton->SetVisible(false);
}