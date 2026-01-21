#include "pch.h"
#include "TutorialStep.h"

#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"


// コンストラクタ
TutorialStep_StartOdenStore::TutorialStep_StartOdenStore(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成 
    tutorialStartStoreImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_start_store.png", "Tutorial_Start_Store");     // おでん屋さんの店主になったよ
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
    tutorialTakeIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_take_ingredient.png", "Tutorial_TakeOdenIngredient");    // まずはダイコンを取ってみよう
    tutorialTakeIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialTakeIngredientImage->SetSize({ 700, 550 });
    tutorialTakeIngredientImage->SetVisible(false);
    uiManager->Add(tutorialTakeIngredientImage);

    // チュートリアル画像の作成  操作方法説明
    tutorialOperateImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_click_oden.png", "Tutorial_ClickOden");    // マウスの左クリックで取れるよ
    tutorialOperateImage->SetWorldPosition({ 50, 300 });
    tutorialOperateImage->SetSize({ 700, 550 });
    tutorialOperateImage->SetVisible(false);
    uiManager->Add(tutorialOperateImage);

}

// デストラクタ
TutorialStep_TakeOdenIngredient::~TutorialStep_TakeOdenIngredient()
{
    if (tutorialTakeIngredientImage)
    {// 削除通知を出す
        tutorialTakeIngredientImage->MarkPendingKill();
    }
    if (tutorialOperateImage)
    {// 削除通知を出す
        tutorialOperateImage->MarkPendingKill();
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
    auto scene = Scene::GetCurrentScene();

    // ポーズ中はゲーム入力を一切受け付けない
    if (scene->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (scene->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialTakeIngredientImage->SetVisible(false);
        tutorialOperateImage->SetVisible(true);
    }


    // ダイコンを掴んだかどうか
    if (auto daikonIngredient = scene->GetActorManager()->GetActorByName("oden_tutorial_daikon"))
    {
        auto daikon = std::dynamic_pointer_cast<OdenIngredientActor>(daikonIngredient);
        if (daikon && daikon->IsGrabIngredient())
        {// 大根のおでん注文が完了したら次のステップへ
            owner->GetTutorialManager()->ChangeState("SubmitOdenIngredient");
        }
    }


}

// ステージから出ていくときのメソッド
void TutorialStep_TakeOdenIngredient::Exit()
{
    tutorialTakeIngredientImage->SetVisible(false);
    tutorialOperateImage->SetVisible(false);
}




// コンストラクタ
TutorialStep_SubmitOdenIngredient::TutorialStep_SubmitOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_grab.png", "Tutorial_TakeOdenIngredient");
    tutorialSubmitIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialSubmitIngredientImage->SetSize({ 700, 550 });
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

}

// デストラクタ
TutorialStep_SubmitOdenIngredient::~TutorialStep_SubmitOdenIngredient()
{
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
}


// ステートに入った時のメソッド
void TutorialStep_SubmitOdenIngredient::Enter()
{
    tutorialSubmitIngredientImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_SubmitOdenIngredient::Execute(float deltaTime)
{
    auto scene = Scene::GetCurrentScene();

    // ポーズ中はゲーム入力を一切受け付けない
    if (scene->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (scene->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialSubmitIngredientImage->SetVisible(false);
    }

    if (auto daikonOrder = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_Daikon"))
    {
        auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(daikonOrder);
        if (odenBubble && odenBubble->IsCompleted())
        {// 大根のおでん注文が完了したら次のステップへ
            owner->GetTutorialManager()->ChangeState("SubmitOdenCircleIngredient");
        }
    }


}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
}





// コンストラクタ
TutorialStep_SubmitOdenCircleIngredient::TutorialStep_SubmitOdenCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_circle_oden.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialSubmitIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialSubmitIngredientImage->SetSize({ 700, 550 });
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

}

// デストラクタ
TutorialStep_SubmitOdenCircleIngredient::~TutorialStep_SubmitOdenCircleIngredient()
{
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
}


// ステートに入った時のメソッド
void TutorialStep_SubmitOdenCircleIngredient::Enter()
{
    tutorialSubmitIngredientImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_SubmitOdenCircleIngredient::Execute(float deltaTime)
{
    auto scene = Scene::GetCurrentScene();

    // ポーズ中はゲーム入力を一切受け付けない
    if (scene->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (scene->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialSubmitIngredientImage->SetVisible(false);
    }

    //if (auto daikonOrder = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_Daikon"))
    //{
    //    auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(daikonOrder);
    //    if (odenBubble && odenBubble->IsCompleted())
    //    {// 大根のおでん注文が完了したら次のステップへ
    //        owner->GetTutorialManager()->ChangeState("ServeOdenToCustomer");
    //    }
    //}


}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenCircleIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
}

