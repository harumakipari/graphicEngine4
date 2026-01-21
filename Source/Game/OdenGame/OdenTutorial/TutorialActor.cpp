#include "pch.h"
#include "TutorialActor.h"
#include "TutorialStep.h"
#include "TutorialManager.h"

void TutorialActor::Initialize(const Transform& transform)
{
    tutorialManager = std::make_unique<TutorialManager>();
    // 各ステートを登録
    tutorialManager->RegisterState(std::make_unique<TutorialStep_StartOdenStore>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_TakeOdenIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SubmitOdenIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SubmitOdenCircleIngredient>(this));

    // 最初のステートに変更
    tutorialManager->ChangeState("StartOdenStore");
}

void TutorialActor::Update(float deltaTime)
{
    // チュートリアルマネージャーの更新
    if (tutorialManager)
    {
        tutorialManager->Update(deltaTime);
    }
}