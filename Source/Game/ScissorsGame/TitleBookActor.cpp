#include "pch.h"
#include "TitleBookActor.h"

#include "ScoreHistoryManager.h"
#include "UI/Game/SceneTransitionManager.h"
#include "TitleScene.h"

void TitleBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);
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

    // 裏表紙のページ
    // ボスステージハイスコア
    {
        std::string scoreParentName = "high_boss_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-1.5f });

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
}

void TitleBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
    // ハイスコアを取得する
    // ステージ１
    int firstHighScore=ScoreHistoryManager::GetHighScore(STAGE_NAME::FIRST);
    firstStageHighScoreDisplay.SetValue(firstHighScore);

    // ボス戦
    int bossHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOSS);
    bossStageHighScoreDisplay.SetValue(bossHighScore);

}

void TitleBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

// 矢印ボタンのUIを作成する
void TitleBookActor::CreateButtonArrow()
{
    auto uiManager = GetOwnerScene()->GetUIManager();
    // 一ページ左
    firstButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow.png", "title_arrow");
    firstButtons.left->SetWorldPosition({ 300, 800 });
    firstButtons.left->SetSize({ 400, 150 });
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

    // 一ページ右
    firstButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/ranking_arrow.png", "ranking_arrow");
    firstButtons.right->SetWorldPosition({ 1000, 800 });
    firstButtons.right->SetSize({ 400, 150 });
    uiManager->Add(firstButtons.right);

    firstButtons.right->onClick = [this]()
        {
            OpenSecondPage(2.0f);
            // ページをめくる音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
        };

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
}
