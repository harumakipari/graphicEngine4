#include "pch.h"
#include "TutorialStep.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"
#include "Engine/Audio/CoreAudio.h"

TutorialStep::TutorialStep(TutorialActor* actor) :owner(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    XMFLOAT2 mousePos = { 1706.0f,389.0f };
    XMFLOAT2 mouseSize = { 90.0f,90.0f };

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
    if (isUpdateMouse)
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
}

void TutorialStep::ShowMouseClick(bool visible)
{
    tutorialMouseClickImage->SetVisible(visible && isMouseClickOn);
    tutorialMouseClickOffImage->SetVisible(visible && !isMouseClickOn);
    isUpdateMouse = true;
}

void TutorialStep::ResetMouseClickBlink()
{
    mouseBlinkTimer = 0.0f;
    isMouseClickOn = false;
    tutorialMouseClickImage->SetVisible(false);
    tutorialMouseClickOffImage->SetVisible(false);
}

// ------------------------------ TutorialStep_MoveStart ------------------------------
//　WASDで移動！
// コンストラクタ
TutorialStep_MoveStart::TutorialStep_MoveStart(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //WASDでいどう
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/wasd_move.png", "wasd_move");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_MoveStart::~TutorialStep_MoveStart()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}


// ステートに入った時のメソッド
void TutorialStep_MoveStart::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;
    startWalk = false; // 歩き始めたかどうか

    //ResetMouseClickBlink();
    //ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_MoveStart::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

#if 0
    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;
#endif // 0

    //UpdateMouseClickBlink(deltaTime);

    if (auto player = owner->GetPlayer())
    {
        if (player->GetStateMachine()->GetStateName() == "Running")
        {// 移動したら
            startWalk = true;
        }
    }

    if (startWalk)
    {// 歩き始めたら経過時間を測る
        elapsedTime += deltaTime;
    }

    if (elapsedTime >= toNextStepInterval)
    {// 次のステップに行くまでの時間が経ったら、
        owner->GetTutorialManager()->ChangeState("TutorialStep_ChargeStart");
    }



#if 0
    //  押した瞬間
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TakeOdenIngredient");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
#endif // 0

}

// ステージから出ていくときのメソッド
void TutorialStep_MoveStart::Exit()
{
    // チュートリアル画像の切り替え
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_ChargeStart ------------------------------
//　「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
// コンストラクタ
TutorialStep_ChargeStart::TutorialStep_ChargeStart(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/left_click_press_release.png", "left_click_press_release");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_ChargeStart::~TutorialStep_ChargeStart()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_ChargeStart::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;
    startDash = false; // 歩き始めたかどうか

}

// ステートで実行するメソッド
void TutorialStep_ChargeStart::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

#if 0
    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;
#endif
    //UpdateMouseClickBlink(deltaTime);

    if (auto player = owner->GetPlayer())
    {
        if (player->GetStateMachine()->GetStateName() == "Dash")
        {// 移動したら
            startDash = true;
        }
    }

    if (startDash)
    {// 歩き始めたら経過時間を測る
        elapsedTime += deltaTime;
    }

    if (elapsedTime >= toNextStepInterval)
    {// 次のステップに行くまでの時間が経ったら、
        owner->GetTutorialManager()->ChangeState("TutorialStep_SpawnStaticEnemy");
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_ChargeStart::Exit()
{
}

// ------------------------------ TutorialStep_SpawnStaticEnemy ------------------------------
//　「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
// コンストラクタ
TutorialStep_SpawnStaticEnemy::TutorialStep_SpawnStaticEnemy(TutorialActor* actor) :TutorialStep(actor)
{

}

// デストラクタ
TutorialStep_SpawnStaticEnemy::~TutorialStep_SpawnStaticEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SpawnStaticEnemy::Enter()
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/left_click_press_release.png", "left_click_press_release");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);

    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;
    startDash = false; // 歩き始めたかどうか

    // 敵を生成する
    auto enemy1 = owner->SpawnEnemy({ 12,0,12 }, YarnEnemyType::Static, false);
    owner->AddTutorialTarget(enemy1);

    auto enemy2 = owner->SpawnEnemy({ 15,0,12 }, YarnEnemyType::Static, false);
    owner->AddTutorialTarget(enemy2);

    auto enemy3 = owner->SpawnEnemy({ 9,0,12 }, YarnEnemyType::Static, false);
    owner->AddTutorialTarget(enemy3);
}

// ステートで実行するメソッド
void TutorialStep_SpawnStaticEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    if (owner->HasPinnedEnemy())
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_TiedEnemy");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SpawnStaticEnemy::Exit()
{
    // チュートリアル画像の切り替え
    tutorialImage->SetVisible(false);
}

// ------------------------------ TutorialStep_TiedEnemy ------------------------------
//　「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
// コンストラクタ
TutorialStep_TiedEnemy::TutorialStep_TiedEnemy(TutorialActor* actor) :TutorialStep(actor)
{

}

// デストラクタ
TutorialStep_TiedEnemy::~TutorialStep_TiedEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_TiedEnemy::Enter()
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //「敵をぬいとめたよ！」「ぬいとめた敵に もう一度ぬいダッシュ！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/tied_enemy_1.png", "tied_enemy_1");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);

    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

}

// ステートで実行するメソッド
void TutorialStep_TiedEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

}

// ステージから出ていくときのメソッド
void TutorialStep_TiedEnemy::Exit()
{
    // チュートリアル画像の切り替え
    tutorialImage->SetVisible(false);
}
