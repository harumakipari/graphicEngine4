#include "pch.h"
#include "TutorialStep.h"

#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"


// コンストラクタ
TutorialStep_StartOdenStore::TutorialStep_StartOdenStore(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成 
    tutorialStartStoreImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_start_store.png", "Tutorial_Start_Store");
    tutorialStartStoreImage->SetWorldPosition({ 50, 300 });
    tutorialStartStoreImage->SetSize({ 700, 550 });
    tutorialStartStoreImage->SetVisible(false);
    uiManager->Add(tutorialStartStoreImage);
}

// デストラクタ
TutorialStep_StartOdenStore::~TutorialStep_StartOdenStore()
{
    if (tutorialStartStoreImage)
    {// 削除通知を出す
        tutorialStartStoreImage->MarkPendingKill();
    }
}


// ステートに入った時のメソッド
void TutorialStep_StartOdenStore::Enter()
{
    tutorialStartStoreImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_StartOdenStore::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;


    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        owner->GetTutorialManager()->ChangeState("TakeOdenIngredient");
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_StartOdenStore::Exit()
{
    // チュートリアル画像の切り替え
    tutorialStartStoreImage->SetVisible(false);
}



// コンストラクタ
TutorialStep_TakeOdenIngredient::TutorialStep_TakeOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialTakeIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_take_ingredient.png", "Tutorial_TakeOdenIngredient");
    tutorialTakeIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialTakeIngredientImage->SetSize({ 700, 550 });
    tutorialTakeIngredientImage->SetVisible(false);
    uiManager->Add(tutorialTakeIngredientImage);

}

// デストラクタ
TutorialStep_TakeOdenIngredient::~TutorialStep_TakeOdenIngredient()
{
    if (tutorialTakeIngredientImage)
    {// 削除通知を出す
        tutorialTakeIngredientImage->MarkPendingKill();
    }
}


// ステートに入った時のメソッド
void TutorialStep_TakeOdenIngredient::Enter()
{
    tutorialTakeIngredientImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_TakeOdenIngredient::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;


    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
    }



}

// ステージから出ていくときのメソッド
void TutorialStep_TakeOdenIngredient::Exit()
{
}