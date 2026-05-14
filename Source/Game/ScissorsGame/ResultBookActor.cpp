#include "pch.h"
#include "ResultBookActor.h"

#include "ScoreCalculator.h"
#include "ScoreHistoryManager.h"
#include "Engine/Scene/Scene.h"
#include "UI/Game/SceneTransitionManager.h"

void ResultBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);

    CreateBookModel("./Data/TeamModels/Title/BookRight.gltf", "./Data/TeamModels/Title/BookMiddle.gltf");

    SetInitPageState(BookPageState::SecondPage);

    // スコアの数字を乗せるページの親
    std::string rightName = rightPage.parentName;

    // スコアの数字モデル　
    {
        std::string scoreParentName = "score_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.8f,-0.1f,-1.7f });
        scoreRoot->SetRelativeScaleDirect({ 0.8f,0.8f,0.8f });

        totalScoreDisplay.Initialize(
            this,
            scoreParentName,
            "total_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, false);
    }

    float subNumberSize = 0.4f;

    // コンボの数字モデル　
    {
        std::string comboParentName = "combo_number_parent";
        auto comboRoot = AddComponent<SceneComponent>(comboParentName, rightName);
        comboRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        comboRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,-0.45f });
        comboRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });

        comboDisplay.Initialize(
            this,
            comboParentName,
            "combo",
            { -0.0f, -0.0f, -0.0f },
            2,
            0.7f, false);
    }

    // ハートボーナス
    {
        std::string heartParentName = "heart_number_parent";
        auto heartRoot = AddComponent<SceneComponent>(heartParentName, rightName);
        heartRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        heartRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,0.1f });
        heartRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        heartDisplay.Initialize(
            this,
            heartParentName,
            "heart",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }

    // 縫い返りボーナス
    {
        std::string scoreParentName = "redirect_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,0.7f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        redirectDisplay.Initialize(
            this,
            scoreParentName,
            "redirect",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }


    // まとめぬいボーナス
    {
        std::string scoreParentName = "gather_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,1.3f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        gatherDisplay.Initialize(
            this,
            scoreParentName,
            "gather",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }


    // クリアタイム
    {
        
    }

    // ニューレコード


    // 裏表紙のページ
    // ランキング 1
    {
        std::string scoreParentName = "ranking_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-1.5f });
        scoreRoot->SetRelativeScaleDirect({ 0.9f,0.9f,0.9f });

        ranking1Display.Initialize(
            this,
            scoreParentName,
            "ranking1",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 2
    {
        std::string scoreParentName = "ranking2_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-0.38f });
        scoreRoot->SetRelativeScaleDirect({ 0.75f,0.75f,0.75f });

        ranking2Display.Initialize(
            this,
            scoreParentName,
            "ranking2",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    float downRankingScale = 0.6f;
    // ランキング 3
    {
        std::string scoreParentName = "ranking3_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,0.55f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking3Display.Initialize(
            this,
            scoreParentName,
            "ranking3",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 4
    {
        std::string scoreParentName = "ranking4_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,1.3f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking4Display.Initialize(
            this,
            scoreParentName,
            "ranking4",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 5
    {
        std::string scoreParentName = "ranking5_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,2.05f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking5Display.Initialize(
            this,
            scoreParentName,
            "ranking5",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }


#if 0

    //  一の位
    numberModel = AddComponent<SkeletalMeshComponent>("numberModel", scoreParentName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_3.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,180.0f });
    numberModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetIsCastShadow(false);

    // スコアの数字モデル　 十の位
    auto numberTenModel = AddComponent<SkeletalMeshComponent>("numberTenModel", scoreParentName);
    numberTenModel->SetModel("./Data/TeamModels/Number/NumberModel_4.gltf", false, false);
    numberTenModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberTenModel->SetRelativeLocationDirect({ -0.7f,-0.0f,-0.0f });
    numberTenModel->SetIsCastShadow(false);
#endif // 0

    // 矢印ボタンのUIを作成する
    CreateButtonArrow();
}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);


#if 1
    totalScoreDisplay.SetValue(97777);
    comboDisplay.SetValue(20);
    heartDisplay.SetValue(1500);
    gatherDisplay.SetValue(1500);
    redirectDisplay.SetValue(1500);

    ranking1Display.SetValue(99000);
    ranking2Display.SetValue(95176);
    ranking3Display.SetValue(15176);
    ranking4Display.SetValue(51176);
    ranking5Display.SetValue(51176);

#else

    // スコアを表示する
    const ResultData& stats = ScoreSystem::GetResultStats();
    int score = stats.totalScore;
    totalScoreDisplay.SetValue(score);

    // 最大コンボ数を表示する
    comboDisplay.SetValue(stats.maxCombo);

    // 残りハートボーナスを表示する
    heartDisplay.SetValue(stats.remainHp * 150);

    // 反射ボーナスを表示する

    // ランキングを取得する
    std::vector<ScoreHistoryManager::Entry> ranking = ScoreHistoryManager::GetTop5(stats.stageName);
    int top1 = ranking[0].score;
    ranking1Display.SetValue(top1);
    int top2 = ranking[1].score;
    ranking2Display.SetValue(top2);
    int top3 = ranking[2].score;
    ranking3Display.SetValue(top3);
    int top4 = ranking[3].score;
    ranking4Display.SetValue(top4);
    int top5 = ranking[3].score;
    ranking5Display.SetValue(top5);
#endif
}

// コントローラー対応の本が開く処理
void ResultBookActor::HandlePadInput()
{
    bool pushA = InputSystem::GetInputState("GamePadA", InputStateMask::Trigger);

    bool pushR = InputSystem::GetInputState("BookRight", InputStateMask::Trigger);
    bool pushL = InputSystem::GetInputState("BookLeft", InputStateMask::Trigger);

    switch (bookState)
    {
    case BookPageState::FirstPage:
        if (pushL)
        {
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
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
        if (pushR)
        {
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        }
        break;
    }
}

void ResultBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

// 矢印ボタンのUIを作成する
void ResultBookActor::CreateButtonArrow()
{
    auto uiManager = GetOwnerScene()->GetUIManager();
    // 一ページ左
    firstButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow.png", "title_arrow");
    firstButtons.left->SetWorldPosition({ 300, 800 });
    firstButtons.left->SetSize({ 400, 150 });
    uiManager->Add(firstButtons.left);

    firstButtons.left->onClick = [this]()
        {
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        };

    // ゲームパッドの画像を設定する
    firstButtons.gamePadLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow.png");


    // 一ページ右
    firstButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/result_arrow.png", "result_arrow");
    firstButtons.right->SetWorldPosition({ 1000, 800 });
    firstButtons.right->SetSize({ 400, 150 });
    uiManager->Add(firstButtons.right);

    firstButtons.right->onClick = [this]()
        {
            OpenSecondPage(2.0f);
            // ページをめくる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
        };

    // ゲームパッドの画像を設定する
    firstButtons.gamePadRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/result_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/result_arrow.png");


    // 二ページ目左
    secondButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/stage_select_arrow.png", "stage_select_arrow");
    secondButtons.left->SetWorldPosition({ 300, 800 });
    secondButtons.left->SetSize({ 400, 150 });
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


    // 二ページ目右
    secondButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow_right.png", "title_arrow_right");
    secondButtons.right->SetWorldPosition({ 1000, 800 });
    secondButtons.right->SetSize({ 400, 150 });
    uiManager->Add(secondButtons.right);

    secondButtons.right->onClick = [this]()
        {
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        };


}