#include "pch.h"
#include "TitleBookActor.h"

#include "ScoreHistoryManager.h"

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


}

void TitleBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
    // ハイスコアを取得する
    int firstHighScore=ScoreHistoryManager::GetHighScore(STAGE_NAME::FIRST);
    firstStageHighScoreDisplay.SetValue(firstHighScore);

    int bossHighScore = ScoreHistoryManager::GetHighScore(STAGE_NAME::BOSS);
    bossStageHighScoreDisplay.SetValue(bossHighScore);

}

void TitleBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

