#include "pch.h"
#include "TutorialStep.h"

#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"
#include "Game/OdenGame/OdenManagers/OdenOrderManager.h"
#include "Game/OdenGame/OdenManagers/OdenSlotManager.h"


// ------------------------------ TutorialStep_StartOdenStore ------------------------------
// おでん屋さんを始める
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



// ------------------------------ TutorialStep_TakeOdenIngredient ------------------------------
// おでんの具材を取る
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


// ------------------------------ TutorialStep_SubmitOdenIngredient ------------------------------
// おでんの具材を渡す
// コンストラクタ
TutorialStep_SubmitOdenIngredient::TutorialStep_SubmitOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_grab.png", "Tutorial_TakeOdenIngredient"); // いいね！そのまま食券のところに持って行こう
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
            owner->GetTutorialManager()->ChangeState("SubmitOdenClear");
        }
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
}


// ------------------------------ TutorialStep_SubmitClearIngredient ------------------------------
// おでんの具材を渡してクリア
// コンストラクタ
TutorialStep_SubmitClearIngredient::TutorialStep_SubmitClearIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialClearSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_submit.png", "Tutorial_TakeOdenCircleIngredient"); // これでおでんの具材を渡せたよ！
    tutorialClearSubmitIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialClearSubmitIngredientImage->SetSize({ 700, 550 });
    tutorialClearSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialClearSubmitIngredientImage);
}

// デストラクタ
TutorialStep_SubmitClearIngredient::~TutorialStep_SubmitClearIngredient()
{
    if (tutorialClearSubmitIngredientImage)
    {// 削除通知を出す
        tutorialClearSubmitIngredientImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SubmitClearIngredient::Enter()
{
    tutorialClearSubmitIngredientImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_SubmitClearIngredient::Execute(float deltaTime)
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
        owner->GetTutorialManager()->ChangeState("SubmitOdenCircleIngredient");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitClearIngredient::Exit()
{
    tutorialClearSubmitIngredientImage->SetVisible(false);
}


// ------------------------------ TutorialStep_SubmitOdenCircleIngredient ------------------------------
// ●のおでんがほしい客が来る
// コンストラクタ
TutorialStep_SubmitOdenCircleIngredient::TutorialStep_SubmitOdenCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 次は、丸いおでんを食べたい人が来たよ
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_circle_oden.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialSubmitIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialSubmitIngredientImage->SetSize({ 700, 550 });
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

    // チュートリアル画像の作成
    // とりあえずダイコンを渡そう
    tutorialAnywayDaikonImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_anyway_daikon.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialAnywayDaikonImage->SetWorldPosition({ 50, 300 });
    tutorialAnywayDaikonImage->SetSize({ 700, 550 });
    tutorialAnywayDaikonImage->SetVisible(false);
    uiManager->Add(tutorialAnywayDaikonImage);
}

// デストラクタ
TutorialStep_SubmitOdenCircleIngredient::~TutorialStep_SubmitOdenCircleIngredient()
{
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
    if (tutorialAnywayDaikonImage)
    {// 削除通知を出す
        tutorialAnywayDaikonImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SubmitOdenCircleIngredient::Enter()
{
    tutorialSubmitIngredientImage->SetVisible(true);

    // 丸いお題を生成
    auto scene = Scene::GetCurrentScene();
    if (auto orderActor = scene->GetActorManager()->GetActorByName("orderManager"))
    {
        auto orderManager = std::dynamic_pointer_cast<OdenOrderManager>(orderActor);
        orderManager->SpawnSpecificOrderBubble(0, "UI_Order_CircleLike");
    }

    // ダイコンを生成  左下のスロットに補充
    if (auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager"))
    {
        auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

        if (auto slotActor = scene->GetActorManager()->GetActorByName("odenSlot_Horizontal_0"))
        {
            auto slot = std::dynamic_pointer_cast<OdenSlotActor>(slotActor);
            slotManager->SupplySpecificIngredientTo(slot, "Daikon");
        }
    }
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
        tutorialAnywayDaikonImage->SetVisible(true);
        tutorialSubmitIngredientImage->SetVisible(false);
    }

    if (auto daikonOrder = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_CircleLike"))
    {
        auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(daikonOrder);
        if (odenBubble && odenBubble->IsCompleted())
        {// 大根のおでん注文が完了したら次のステップへ
            owner->GetTutorialManager()->ChangeState("ClearCircleIngredient");
        }
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenCircleIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
    tutorialAnywayDaikonImage->SetVisible(false);
}

// ------------------------------ TutorialStep_ClearCircleIngredient ------------------------------
// ●のおでんがほしい客をクリア
// コンストラクタ
TutorialStep_ClearCircleIngredient::TutorialStep_ClearCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    // そうそう！ダイコンって上から見ると丸いよね！
    tutorialClearCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_circle.png", "Tutorial_ClearCircleIngredient");
    tutorialClearCircleIngredientImage->SetWorldPosition({ 50, 300 });
    tutorialClearCircleIngredientImage->SetSize({ 700, 550 });
    tutorialClearCircleIngredientImage->SetVisible(false);
    uiManager->Add(tutorialClearCircleIngredientImage);

}

// デストラクタ
TutorialStep_ClearCircleIngredient::~TutorialStep_ClearCircleIngredient()
{
    if (tutorialClearCircleIngredientImage)
    {// 削除通知を出す
        tutorialClearCircleIngredientImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ClearCircleIngredient::Enter()
{
    auto scene = Scene::GetCurrentScene();

    tutorialClearCircleIngredientImage->SetVisible(true);

    // ダイコンを生成  左下のスロットに補充
    if (auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager"))
    {
        auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

        if (auto slotActor = scene->GetActorManager()->GetActorByName("odenSlot_Horizontal_0"))
        {
            auto slot = std::dynamic_pointer_cast<OdenSlotActor>(slotActor);
            slotManager->SupplySpecificIngredientTo(slot, "Daikon");
        }
    }
}

// ステートで実行するメソッド
void TutorialStep_ClearCircleIngredient::Execute(float deltaTime)
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
        tutorialClearCircleIngredientImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("RotateOdenIngredient"); // 次のステップへ
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ClearCircleIngredient::Exit()
{
    tutorialClearCircleIngredientImage->SetVisible(false);
}


// ------------------------------ TutorialStep_RotateOdenIngredient ------------------------------
TutorialStep_RotateOdenIngredient::TutorialStep_RotateOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成
    // おでんって、回ると見え方が変わるんだよー！
    tutorialRotateOdenImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_rotate_oden.png", "Tutorial_RotateOden");
    tutorialRotateOdenImage->SetWorldPosition({ 50, 300 });
    tutorialRotateOdenImage->SetSize({ 700, 550 });
    tutorialRotateOdenImage->SetVisible(false);
    uiManager->Add(tutorialRotateOdenImage);
}

// デストラクタ
TutorialStep_RotateOdenIngredient::~TutorialStep_RotateOdenIngredient()
{
    if (tutorialRotateOdenImage)
    {// 削除通知を出す
        tutorialRotateOdenImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_RotateOdenIngredient::Enter()
{
    auto scene = Scene::GetCurrentScene();

    tutorialRotateOdenImage->SetVisible(true);

#if 0
    // ダイコンを生成  左下のスロットに補充
    if (auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager"))
    {
        auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

        if (auto slotActor = scene->GetActorManager()->GetActorByName("odenSlot_Horizontal_0"))
        {
            auto slot = std::dynamic_pointer_cast<OdenSlotActor>(slotActor);
            slotManager->SupplySpecificIngredientTo(slot, "Daikon");
        }
    }
#endif // 0
}

// ステートで実行するメソッド
void TutorialStep_RotateOdenIngredient::Execute(float deltaTime)
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
        //tutorialRotateOdenImage->SetVisible(true);
        // ダイコンを生成  左下のスロットに補充
        if (auto daikonActor = scene->GetActorManager()->GetActorByName("OdenIngredient_Specific_Daikon"))
        {
            auto daikon = std::dynamic_pointer_cast<OdenIngredientActor>(daikonActor);
            daikon->RotateHorizontal(); // 横回転させる
        }
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_RotateOdenIngredient::Exit()
{
    tutorialRotateOdenImage->SetVisible(false);
}