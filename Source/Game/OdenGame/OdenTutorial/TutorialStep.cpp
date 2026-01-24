#include "pch.h"
#include "TutorialStep.h"

#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"
#include "Engine/Audio/CoreAudio.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"
#include "Game/OdenGame/OdenManagers/OdenOrderManager.h"
#include "Game/OdenGame/OdenManagers/OdenSlotManager.h"


TutorialStep::TutorialStep(TutorialActor* actor) :owner(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    XMFLOAT2 mousePos = { 1605.0f,340.0f };
    XMFLOAT2 mouseSize = { 120.0f,120.0f };

    // チュートリアル画像の作成 
    tutorialMouseClickImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/mouseClick.png", "Tutorial_Mouse_Click");     // マウスのクリック
    tutorialMouseClickImage->SetWorldPosition(mousePos);
    tutorialMouseClickImage->SetSize(mouseSize);
    tutorialMouseClickImage->zOrder = 5;
    tutorialMouseClickImage->SetVisible(false);
    uiManager->Add(tutorialMouseClickImage);

    // チュートリアル画像の作成 
    tutorialMouseClickOffImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/mouseClick_off.png", "Tutorial_Mouse_Click_Off");     // マウスのクリックオフ
    tutorialMouseClickOffImage->SetWorldPosition(mousePos);
    tutorialMouseClickOffImage->SetSize(mouseSize);
    tutorialMouseClickOffImage->SetVisible(false);
    tutorialMouseClickOffImage->zOrder = 5;
    uiManager->Add(tutorialMouseClickOffImage);
}


void TutorialStep::UpdateMouseClickBlink(float deltaTime)
{
    mouseBlinkTimer += deltaTime;

    if (mouseBlinkTimer >= mouseBlinkInterval)
    {
        mouseBlinkTimer = 0.0f;
        isMouseClickOn = !isMouseClickOn;

        tutorialMouseClickImage->SetVisible(isMouseClickOn);
        tutorialMouseClickOffImage->SetVisible(!isMouseClickOn);
    }
}

void TutorialStep::ShowMouseClick(bool visible)
{
    tutorialMouseClickImage->SetVisible(visible && isMouseClickOn);
    tutorialMouseClickOffImage->SetVisible(visible && !isMouseClickOn);
}

void TutorialStep::ResetMouseClickBlink()
{
    mouseBlinkTimer = 0.0f;
    isMouseClickOn = false;
    tutorialMouseClickImage->SetVisible(false);
    tutorialMouseClickOffImage->SetVisible(false);
}



// ------------------------------ TutorialStep_StartOdenStore ------------------------------
// おでん屋さんを始める
// コンストラクタ
TutorialStep_StartOdenStore::TutorialStep_StartOdenStore(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成 
    tutorialStartStoreImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_start_store.png", "Tutorial_Start_Store");     // おでん屋さんの店主になったよ
    tutorialStartStoreImage->SetWorldPosition(imagePos);
    tutorialStartStoreImage->SetSize(imageSize);
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

    ResetMouseClickBlink();
    ShowMouseClick(true);
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        owner->GetTutorialManager()->ChangeState("TakeOdenIngredient");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav");
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_StartOdenStore::Exit()
{
    // チュートリアル画像の切り替え
    tutorialStartStoreImage->SetVisible(false);
    ShowMouseClick(false);
}

// ------------------------------ TutorialStep_TakeOdenIngredient ------------------------------
// おでんの具材を取る
// コンストラクタ
TutorialStep_TakeOdenIngredient::TutorialStep_TakeOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialTakeIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_take_ingredient.png", "Tutorial_TakeOdenIngredient");    // まずはダイコンを取ってみよう
    tutorialTakeIngredientImage->SetWorldPosition(imagePos);
    tutorialTakeIngredientImage->SetSize(imageSize);
    tutorialTakeIngredientImage->SetVisible(false);
    uiManager->Add(tutorialTakeIngredientImage);

    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_grab.png", "Tutorial_TakeOdenIngredient"); // いいね！そのまま食券のところに持って行こう
    tutorialSubmitIngredientImage->SetWorldPosition(imagePos);
    tutorialSubmitIngredientImage->SetSize(imageSize);
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

    XMFLOAT2 bubbleSize = { 400.0f,150.0f };
    XMFLOAT2 bubblePos = { 80.0f,130.0f };
    tutorialReleaseMouseImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_release_mouse.png", "tutorial_release_mouse");    // マウスを離そう
    tutorialReleaseMouseImage->SetWorldPosition(bubblePos);
    tutorialReleaseMouseImage->SetSize(bubbleSize);
    tutorialReleaseMouseImage->SetVisible(false);
    uiManager->Add(tutorialReleaseMouseImage);
}

// デストラクタ
TutorialStep_TakeOdenIngredient::~TutorialStep_TakeOdenIngredient()
{
    if (tutorialTakeIngredientImage)
    {// 削除通知を出す
        tutorialTakeIngredientImage->MarkPendingKill();
    }
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
    if (tutorialReleaseMouseImage)
    {// 削除通知を出す
        tutorialReleaseMouseImage->MarkPendingKill();
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

    ////  押した瞬間
    //if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    //{
    //    tutorialTakeIngredientImage->SetVisible(false);
    //    tutorialOperateImage->SetVisible(true);
    //}
#if 0
    // ダイコンを掴んだかどうか
    if (auto daikonIngredient = scene->GetActorManager()->GetActorByName("oden_tutorial_daikon"))
    {
        auto daikon = std::dynamic_pointer_cast<OdenIngredientActor>(daikonIngredient);
        if (daikon && daikon->IsGrabIngredient())
        {// 大根のおでん注文が完了したら次のステップへ
            owner->GetTutorialManager()->ChangeState("SubmitOdenIngredient");
        }
    }
#else
    // ダイコンを掴んだかどうか
    if (auto grabbed = owner->GetGrabbedIngredient())
    {
        if (grabbed->GetIngredientType() == EOdenType::Daikon)
        {
            tutorialTakeIngredientImage->SetVisible(false);
            tutorialSubmitIngredientImage->SetVisible(true);
        }

        if (grabbed->IsHoveringOrder())
        {// オーダーの上にダイコンが来たら
            Logger::Log(U8("ダイコンがオーダーの上にある"));
            tutorialReleaseMouseImage->SetVisible(true);
        }
        else
        {
            Logger::Log(U8("ダイコンがオーダーの上にない"));
            tutorialReleaseMouseImage->SetVisible(false);
        }
    }
    else
    {
        tutorialReleaseMouseImage->SetVisible(false);
    }

    if (auto daikonOrder = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_Daikon"))
    {
        auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(daikonOrder);
        if (odenBubble && odenBubble->IsCompleted())
        {// 大根のおでん注文が完了したら次のステップへ
            tutorialReleaseMouseImage->SetVisible(false);
            owner->GetTutorialManager()->ChangeState("SubmitOdenClear");
            //owner->GetTutorialManager()->ChangeState("SubmitOdenIngredient");
        }
    }

#endif // 0

}

// ステージから出ていくときのメソッド
void TutorialStep_TakeOdenIngredient::Exit()
{
    tutorialTakeIngredientImage->SetVisible(false);
    tutorialSubmitIngredientImage->SetVisible(false);
    tutorialReleaseMouseImage->SetVisible(false);
}

// 掴んではいけない食材の時の処理
void TutorialStep_TakeOdenIngredient::OnDeniedGrab(std::shared_ptr<Actor> ingredient)
{
    if (auto ingredientActor = std::dynamic_pointer_cast<OdenIngredientActor>(ingredient))
    {
        owner->ShowBalloonNearIngredient(ingredientActor);
    }
}

// 掴める食材の時の処理
void TutorialStep_TakeOdenIngredient::OnAllowGrab(std::shared_ptr<Actor> ingredient)
{
    owner->HideAllBubbles();
}


// ------------------------------ TutorialStep_SubmitClearIngredient ------------------------------
// おでんの具材を渡してクリア
// コンストラクタ
TutorialStep_SubmitClearIngredient::TutorialStep_SubmitClearIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    tutorialClearSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_submit.png", "Tutorial_TakeOdenCircleIngredient"); // これでおでんの具材を渡せたよ！
    tutorialClearSubmitIngredientImage->SetWorldPosition(imagePos);
    tutorialClearSubmitIngredientImage->SetSize(imageSize);
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

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_SubmitClearIngredient::Execute(float deltaTime)
{
    auto scene = Scene::GetCurrentScene();

    // ポーズ中はゲーム入力を一切受け付けない
    if (scene->IsPaused())
        return;

    UpdateMouseClickBlink(deltaTime);

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
        owner->GetTutorialManager()->ChangeState("ComeOdenCircleIngredient");
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitClearIngredient::Exit()
{
    tutorialClearSubmitIngredientImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_ComeOdenCircleIngredient ------------------------------
// ●のおでんがほしい客が来る
// コンストラクタ
TutorialStep_ComeOdenCircleIngredient::TutorialStep_ComeOdenCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 次は、丸いおでんを食べたい人が来たよ
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_circle_oden.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialSubmitIngredientImage->SetWorldPosition(imagePos);
    tutorialSubmitIngredientImage->SetSize(imageSize);
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

}

// デストラクタ
TutorialStep_ComeOdenCircleIngredient::~TutorialStep_ComeOdenCircleIngredient()
{
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ComeOdenCircleIngredient::Enter()
{
    tutorialSubmitIngredientImage->SetVisible(true);

    // 丸いお題を生成
    auto scene = Scene::GetCurrentScene();
    if (auto orderActor = scene->GetActorManager()->GetActorByName("orderManager"))
    {
        auto orderManager = std::dynamic_pointer_cast<OdenOrderManager>(orderActor);
        orderManager->SpawnSpecificOrderBubble(0, "UI_Order_CircleLike");
    }

    ResetMouseClickBlink();
    ShowMouseClick(true);

    // ダイコンを生成  
    auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager");

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    if (!slotManager)
        return;

    // 空スロットを探す
    for (auto& weakSlot : slotManager->GetSlots())
    {
        auto slot = weakSlot.lock();
        if (!slot)
            continue;

        if (!slot->GetIngredient())
        {
            slotManager->SupplySpecificIngredientTo(slot, "Daikon");
            break; // 1個でいいなら break
        }
    }

}

// ステートで実行するメソッド
void TutorialStep_ComeOdenCircleIngredient::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialSubmitIngredientImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("SubmitOdenCircleIngredient");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ComeOdenCircleIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_SubmitOdenCircleIngredient ------------------------------
// ●のおでんを渡す
// コンストラクタ
TutorialStep_SubmitOdenCircleIngredient::TutorialStep_SubmitOdenCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // まるっぽいおでんを渡そう！
    tutorialAnywayDaikonImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_submit_circle.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialAnywayDaikonImage->SetWorldPosition(imagePos);
    tutorialAnywayDaikonImage->SetSize(imageSize);
    tutorialAnywayDaikonImage->SetVisible(false);
    uiManager->Add(tutorialAnywayDaikonImage);

    // いいね！そのまま食券のところに持って行こう
    tutorialSubmitCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_grab.png", "Tutorial_TakeOdenIngredient");
    tutorialSubmitCircleIngredientImage->SetWorldPosition(imagePos);
    tutorialSubmitCircleIngredientImage->SetSize(imageSize);
    tutorialSubmitCircleIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitCircleIngredientImage);

    XMFLOAT2 bubbleSize = { 400.0f,150.0f };
    XMFLOAT2 bubblePos = { 80.0f,130.0f };
    tutorialReleaseMouseImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_release_mouse.png", "tutorial_release_mouse");    // マウスを離そう
    tutorialReleaseMouseImage->SetWorldPosition(bubblePos);
    tutorialReleaseMouseImage->SetSize(bubbleSize);
    tutorialReleaseMouseImage->SetVisible(false);
    uiManager->Add(tutorialReleaseMouseImage);

}

// デストラクタ
TutorialStep_SubmitOdenCircleIngredient::~TutorialStep_SubmitOdenCircleIngredient()
{
    if (tutorialAnywayDaikonImage)
    {// 削除通知を出す
        tutorialAnywayDaikonImage->MarkPendingKill();
    }
    if (tutorialSubmitCircleIngredientImage)
    {// 削除通知を出す
        tutorialSubmitCircleIngredientImage->MarkPendingKill();
    }
    if (tutorialReleaseMouseImage)
    {// 削除通知を出す
        tutorialReleaseMouseImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SubmitOdenCircleIngredient::Enter()
{
    tutorialAnywayDaikonImage->SetVisible(true);
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


    // 丸を掴んだかどうか
    if (auto grabbed = owner->GetGrabbedIngredient())
    {
        if (grabbed->GetCurrentShape().category == EOdenShapeCategory::RoundLike)
        {
            tutorialAnywayDaikonImage->SetVisible(false);
            tutorialSubmitCircleIngredientImage->SetVisible(true);
        }

        if (grabbed->IsHoveringOrder())
        {// オーダーの上にダイコンが来たら
            Logger::Log(U8("丸がオーダーの上にある"));
            tutorialReleaseMouseImage->SetVisible(true);
        }
        else
        {
            Logger::Log(U8("丸ががオーダーの上にない"));
            tutorialReleaseMouseImage->SetVisible(false);
        }
    }
    else
    {
        tutorialReleaseMouseImage->SetVisible(false);
    }


    if (auto order = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_CircleLike"))
    {
        auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(order);
        if (odenBubble && odenBubble->IsCompleted())
        {// まるの形のおでん注文が完了したら次のステップへ
            tutorialReleaseMouseImage->SetVisible(false);
            owner->GetTutorialManager()->ChangeState("ClearCircleIngredient");
        }
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenCircleIngredient::Exit()
{
    tutorialAnywayDaikonImage->SetVisible(false);
    tutorialReleaseMouseImage->SetVisible(false);
    tutorialSubmitCircleIngredientImage->SetVisible(false);
}

// 掴んではいけない食材の時の処理
void TutorialStep_SubmitOdenCircleIngredient::OnDeniedGrab(std::shared_ptr<Actor> ingredient)
{
    if (auto ingredientActor = std::dynamic_pointer_cast<OdenIngredientActor>(ingredient))
    {
        owner->ShowCircleBallonNearIngredient(ingredientActor);
    }
}

// 掴める食材の時の処理
void TutorialStep_SubmitOdenCircleIngredient::OnAllowGrab(std::shared_ptr<Actor> ingredient)
{
    owner->HideAllBubbles();
}


// ------------------------------ TutorialStep_ClearCircleIngredient ------------------------------
// ●のおでんがほしい客をクリア
// コンストラクタ
TutorialStep_ClearCircleIngredient::TutorialStep_ClearCircleIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    // 注文通りのおでんが渡せたね！
    tutorialClearCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_circle.png", "Tutorial_ClearCircleIngredient");
    tutorialClearCircleIngredientImage->SetWorldPosition(imagePos);
    tutorialClearCircleIngredientImage->SetSize(imageSize);
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

    ResetMouseClickBlink();
    ShowMouseClick(true);

    // ダイコンを生成  
    auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager");

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    if (!slotManager)
        return;

    // 空スロットを探す
    for (auto& weakSlot : slotManager->GetSlots())
    {
        auto slot = weakSlot.lock();
        if (!slot)
            continue;

        if (!slot->GetIngredient())
        {
            slotManager->SupplySpecificIngredientTo(slot, "Daikon");
            break; // 1個でいいなら break
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

    UpdateMouseClickBlink(deltaTime);

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
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_RotateOdenIngredient ------------------------------
// 回るとおでんの見え方が変わるね！
TutorialStep_RotateOdenIngredient::TutorialStep_RotateOdenIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成
    // 回るとおでんの見え方が変わるね！
    tutorialRotateOdenImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_rotate_oden.png", "Tutorial_RotateOden");
    tutorialRotateOdenImage->SetWorldPosition(imagePos);
    tutorialRotateOdenImage->SetSize(imageSize);
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

    // スロットの回転を開始する
    if (auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager"))
    {
        auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);
        if (slotManager)
        {
            slotManager->SetRotationEnabled(true);
            slotManager->SetBeatInterval(2.0);    // 半分に設定する
        }
    }

    ResetMouseClickBlink();
    ShowMouseClick(true);

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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialRotateOdenImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("ComeOdenSquareIngredient"); // 次のステップへ
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_RotateOdenIngredient::Exit()
{
    tutorialRotateOdenImage->SetVisible(false);
    ShowMouseClick(false);
}

// ------------------------------ TutorialStep_ComeOdenSquareIngredient ------------------------------
// 四角のおでんがほしい客が来る
// コンストラクタ
TutorialStep_ComeOdenSquareIngredient::TutorialStep_ComeOdenSquareIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 次は、四角いおでんを食べたい人が来たよ
    tutorialSubmitIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_square_oden.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialSubmitIngredientImage->SetWorldPosition(imagePos);
    tutorialSubmitIngredientImage->SetSize(imageSize);
    tutorialSubmitIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitIngredientImage);

}

// デストラクタ
TutorialStep_ComeOdenSquareIngredient::~TutorialStep_ComeOdenSquareIngredient()
{
    if (tutorialSubmitIngredientImage)
    {// 削除通知を出す
        tutorialSubmitIngredientImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ComeOdenSquareIngredient::Enter()
{
    tutorialSubmitIngredientImage->SetVisible(true);

    // 四角いお題を生成
    auto scene = Scene::GetCurrentScene();
    if (auto orderActor = scene->GetActorManager()->GetActorByName("orderManager"))
    {
        auto orderManager = std::dynamic_pointer_cast<OdenOrderManager>(orderActor);
        orderManager->SpawnSpecificOrderBubble(0, "UI_Order_SquareLike");
    }

    ResetMouseClickBlink();
    ShowMouseClick(true);


}

// ステートで実行するメソッド
void TutorialStep_ComeOdenSquareIngredient::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialSubmitIngredientImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("SubmitOdenSquareIngredient");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ComeOdenSquareIngredient::Exit()
{
    tutorialSubmitIngredientImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_SubmitOdenSquareIngredient ------------------------------
// 四角のおでんを渡す
// コンストラクタ
TutorialStep_SubmitOdenSquareIngredient::TutorialStep_SubmitOdenSquareIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 四角っぽいおでんを渡そう！
    tutorialAnywayDaikonImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_submit_square.png", "Tutorial_TakeOdenCircleIngredient");
    tutorialAnywayDaikonImage->SetWorldPosition(imagePos);
    tutorialAnywayDaikonImage->SetSize(imageSize);
    tutorialAnywayDaikonImage->SetVisible(false);
    uiManager->Add(tutorialAnywayDaikonImage);

    // いいね！そのまま食券のところに持って行こう
    tutorialSubmitCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_grab.png", "Tutorial_TakeOdenIngredient");
    tutorialSubmitCircleIngredientImage->SetWorldPosition(imagePos);
    tutorialSubmitCircleIngredientImage->SetSize(imageSize);
    tutorialSubmitCircleIngredientImage->SetVisible(false);
    uiManager->Add(tutorialSubmitCircleIngredientImage);

    XMFLOAT2 bubbleSize = { 400.0f,150.0f };
    XMFLOAT2 bubblePos = { 80.0f,130.0f };
    tutorialReleaseMouseImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_release_mouse.png", "tutorial_release_mouse");    // マウスを離そう
    tutorialReleaseMouseImage->SetWorldPosition(bubblePos);
    tutorialReleaseMouseImage->SetSize(bubbleSize);
    tutorialReleaseMouseImage->SetVisible(false);
    uiManager->Add(tutorialReleaseMouseImage);

}

// デストラクタ
TutorialStep_SubmitOdenSquareIngredient::~TutorialStep_SubmitOdenSquareIngredient()
{
    if (tutorialAnywayDaikonImage)
    {// 削除通知を出す
        tutorialAnywayDaikonImage->MarkPendingKill();
    }
    if (tutorialSubmitCircleIngredientImage)
    {// 削除通知を出す
        tutorialSubmitCircleIngredientImage->MarkPendingKill();
    }
    if (tutorialReleaseMouseImage)
    {// 削除通知を出す
        tutorialReleaseMouseImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SubmitOdenSquareIngredient::Enter()
{
    tutorialAnywayDaikonImage->SetVisible(true);
}

// ステートで実行するメソッド
void TutorialStep_SubmitOdenSquareIngredient::Execute(float deltaTime)
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


    // 丸を掴んだかどうか
    if (auto grabbed = owner->GetGrabbedIngredient())
    {
        if (grabbed->GetCurrentShape().category == EOdenShapeCategory::SquareLike)
        {
            tutorialAnywayDaikonImage->SetVisible(false);
            tutorialSubmitCircleIngredientImage->SetVisible(true);
        }

        if (grabbed->IsHoveringOrder())
        {// オーダーの上にダイコンが来たら
            Logger::Log(U8("四角がオーダーの上にある"));
            tutorialReleaseMouseImage->SetVisible(true);
        }
        else
        {
            Logger::Log(U8("四角ががオーダーの上にない"));
            tutorialReleaseMouseImage->SetVisible(false);
        }
    }
    else
    {
        tutorialReleaseMouseImage->SetVisible(false);
    }


    if (auto order = scene->GetActorManager()->GetActorByName("TutorialOdenBubble_UI_Order_SquareLike"))
    {
        auto odenBubble = std::dynamic_pointer_cast<OdenBubbleActor>(order);
        if (odenBubble && odenBubble->IsCompleted())
        {// 四角の形のおでん注文が完了したら次のステップへ
            tutorialReleaseMouseImage->SetVisible(false);
            owner->GetTutorialManager()->ChangeState("ClearSquareIngredient");
        }
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SubmitOdenSquareIngredient::Exit()
{
    tutorialAnywayDaikonImage->SetVisible(false);
    tutorialReleaseMouseImage->SetVisible(false);
    tutorialSubmitCircleIngredientImage->SetVisible(false);
}

// 掴んではいけない食材の時の処理
void TutorialStep_SubmitOdenSquareIngredient::OnDeniedGrab(std::shared_ptr<Actor> ingredient)
{
    if (auto ingredientActor = std::dynamic_pointer_cast<OdenIngredientActor>(ingredient))
    {
        owner->ShowSquareBallonNearIngredient(ingredientActor);
    }
}

// 掴める食材の時の処理
void TutorialStep_SubmitOdenSquareIngredient::OnAllowGrab(std::shared_ptr<Actor> ingredient)
{
    owner->HideAllBubbles();
}


// ------------------------------ TutorialStep_ClearSquareIngredient ------------------------------
// 四角のおでんがほしい客をクリア
// コンストラクタ
TutorialStep_ClearSquareIngredient::TutorialStep_ClearSquareIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    // 注文通りのおでんが渡せたね！
    tutorialClearCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_circle.png", "Tutorial_ClearCircleIngredient");
    tutorialClearCircleIngredientImage->SetWorldPosition(imagePos);
    tutorialClearCircleIngredientImage->SetSize(imageSize);
    tutorialClearCircleIngredientImage->SetVisible(false);
    uiManager->Add(tutorialClearCircleIngredientImage);

}

// デストラクタ
TutorialStep_ClearSquareIngredient::~TutorialStep_ClearSquareIngredient()
{
    if (tutorialClearCircleIngredientImage)
    {// 削除通知を出す
        tutorialClearCircleIngredientImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ClearSquareIngredient::Enter()
{
    auto scene = Scene::GetCurrentScene();

    tutorialClearCircleIngredientImage->SetVisible(true);

    ResetMouseClickBlink();
    ShowMouseClick(true);

    // ダイコンを生成  
    auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager");

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    if (!slotManager)
        return;

    // 空スロットを探す
    for (auto& weakSlot : slotManager->GetSlots())
    {
        auto slot = weakSlot.lock();
        if (!slot)
            continue;

        if (!slot->GetIngredient())
        {
            slotManager->SupplySpecificIngredientTo(slot, "Chikuwa");
            break; // 1個でいいなら break
        }
    }
}

// ステートで実行するメソッド
void TutorialStep_ClearSquareIngredient::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialClearCircleIngredientImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("SwapIngredient"); // 次のステップへ
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ClearSquareIngredient::Exit()
{
    tutorialClearCircleIngredientImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_SwapIngredient ------------------------------
// 食材のスワップ
// コンストラクタ
TutorialStep_SwapIngredient::TutorialStep_SwapIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成 
    // 下の段のおでんを上に移してみよう！
    tutorialClearCircleIngredientImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_swap_ingredient.png", "tutorial_swap_ingredient");
    tutorialClearCircleIngredientImage->SetWorldPosition(imagePos);
    tutorialClearCircleIngredientImage->SetSize(imageSize);
    tutorialClearCircleIngredientImage->SetVisible(false);
    uiManager->Add(tutorialClearCircleIngredientImage);

    // チュートリアル画像の作成 
    // 矢印
    tutorialArrowImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_arrow.png", "tutorial_arrow");
    tutorialArrowImage->SetWorldPosition({ 10.0f,10.0f });
    tutorialArrowImage->SetSize({ 100.0f,600.0f });
    tutorialArrowImage->SetVisible(false);
    uiManager->Add(tutorialArrowImage);

}

// デストラクタ
TutorialStep_SwapIngredient::~TutorialStep_SwapIngredient()
{
    if (tutorialClearCircleIngredientImage)
    {// 削除通知を出す
        tutorialClearCircleIngredientImage->MarkPendingKill();
    }
    if (tutorialArrowImage)
    {// 削除通知を出す
        tutorialArrowImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SwapIngredient::Enter()
{
    auto scene = Scene::GetCurrentScene();

    tutorialClearCircleIngredientImage->SetVisible(true);
    tutorialArrowImage->SetVisible(true);

    // ダイコンを生成  
    auto slotManagerActor = scene->GetActorManager()->GetActorByName("slotManager");

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    if (!slotManager)
        return;

    // 空スロットを探す
    for (auto& weakSlot : slotManager->GetSlots())
    {
        auto slot = weakSlot.lock();
        if (!slot)
            continue;

        if (!slot->GetIngredient())
        {
            slotManager->SupplySpecificIngredientTo(slot, "Egg");
            break; // 1個でいいなら break
        }
    }
}

// ステートで実行するメソッド
void TutorialStep_SwapIngredient::Execute(float deltaTime)
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
        //tutorialClearCircleIngredientImage->SetVisible(false);
        //owner->GetTutorialManager()->ChangeState("ClearSwapIngredient"); // 次のステップへ
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SwapIngredient::Exit()
{
    tutorialClearCircleIngredientImage->SetVisible(false);
}



// ------------------------------ TutorialStep_ClearSwapIngredient ------------------------------
// スワップできたあとに呼ぶ処理
// コンストラクタ
TutorialStep_ClearSwapIngredient::TutorialStep_ClearSwapIngredient(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // おでんが入れ替わって回り方が変わるよ
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear_swap.png", "tutorial_clear_swap");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_ClearSwapIngredient::~TutorialStep_ClearSwapIngredient()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ClearSwapIngredient::Enter()
{
    tutorialImage->SetVisible(true);

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_ClearSwapIngredient::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("IntroduceShape");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ClearSwapIngredient::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_IntroduceShape ------------------------------
// スワップできたあとに呼ぶ処理
// コンストラクタ
TutorialStep_IntroduceShape::TutorialStep_IntroduceShape(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 他にも。。
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_introduce_shape.png", "tutorial_introduce_shape");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_IntroduceShape::~TutorialStep_IntroduceShape()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_IntroduceShape::Enter()
{
    tutorialImage->SetVisible(true);

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_IntroduceShape::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialImage->SetVisible(false);
        owner->GetTutorialManager()->ChangeState("ClearTutorial");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_IntroduceShape::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}

// ------------------------------ TutorialStep_ClearTutorial ------------------------------
// スワップできたあとに呼ぶ処理
// コンストラクタ
TutorialStep_ClearTutorial::TutorialStep_ClearTutorial(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // チュートリアル終わり
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_clear.png", "tutorial_clear");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_ClearTutorial::~TutorialStep_ClearTutorial()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ClearTutorial::Enter()
{
    tutorialImage->SetVisible(true);

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_ClearTutorial::Execute(float deltaTime)
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

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        tutorialImage->SetVisible(false);
        // シーン遷移する
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("type", types[rand() % 2]) });
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_ClearTutorial::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}