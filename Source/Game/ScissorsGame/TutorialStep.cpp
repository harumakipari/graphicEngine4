#include "pch.h"
#include "TutorialStep.h"

#include "EnemyBase.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "TutorialActor.h"
#include "Engine/Input/InputSystem.h"
#include "TutorialManager.h"
#include "Engine/Audio/CoreAudio.h"

TutorialStep::TutorialStep(TutorialActor* actor) :owner(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    XMFLOAT2 mousePos = { 870.0f,200.0f };
    XMFLOAT2 mouseSize = { 90.0f,90.0f };

    // コントローラー対応用
    controlButtonOnImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/A.png");
    controlButtonOffImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/A_off.png");
    // キーボード対応用
    keyBoardButtonOnImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/mouseClick.png");
    keyBoardButtonOffImage = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/mouseClick_off.png");

    // チュートリアル画像の作成 
    tutorialMouseClickImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/mouseClick.png", "Tutorial_Mouse_Click");     // マウスのクリック
    tutorialMouseClickImage->SetWorldPosition(mousePos);
    tutorialMouseClickImage->SetSize(mouseSize);
    tutorialMouseClickImage->zOrder = 5;
    tutorialMouseClickImage->SetVisible(false);
    uiManager->Add(tutorialMouseClickImage);

    // チュートリアル画像の作成 
    tutorialMouseClickOffImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/mouseClick_off.png", "Tutorial_Mouse_Click_Off");     // マウスのクリックオフ
    tutorialMouseClickOffImage->SetWorldPosition(mousePos);
    tutorialMouseClickOffImage->SetSize(mouseSize);
    tutorialMouseClickOffImage->SetVisible(false);
    tutorialMouseClickOffImage->zOrder = 5;
    uiManager->Add(tutorialMouseClickOffImage);

    isUseRedirect = false;
    isRedirectKillEnemy = false;
    isBonusKill5Enemy = false;
}


void TutorialStep::UpdateMouseClickBlink(float deltaTime)
{
    if (InputSystem::IsGamepadConnected())
    {//　コントローラー対応
        tutorialMouseClickImage->SetTexture(controlButtonOnImage);
        tutorialMouseClickOffImage->SetTexture(controlButtonOffImage);
    }
    else
    {
        tutorialMouseClickImage->SetTexture(keyBoardButtonOnImage);
        tutorialMouseClickOffImage->SetTexture(keyBoardButtonOffImage);
    }

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

    // コントローラー対応用
    controlTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/control_move.png");
    // キーボード対応用
    keyBoardTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/wasd_move.png");


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

    if (InputSystem::IsGamepadConnected())
    {//　コントローラー対応
        tutorialImage->SetTexture(controlTex);
    }
    else
    {
        tutorialImage->SetTexture(keyBoardTex);
    }

#if 0
    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;
#endif // 0


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
    UpdateMouseClickBlink(deltaTime);


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

    // コントローラー対応用
    controlTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/right_stick.png");
    // キーボード対応用
    keyBoardTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/left_click_press_release.png");


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

    if (InputSystem::IsGamepadConnected())
    {//　コントローラー対応
        tutorialImage->SetTexture(controlTex);
    }
    else
    {
        tutorialImage->SetTexture(keyBoardTex);
    }


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
    tutorialImage->SetVisible(false);
}

// ------------------------------ TutorialStep_SpawnStaticEnemy ------------------------------
//　「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
// コンストラクタ
TutorialStep_SpawnStaticEnemy::TutorialStep_SpawnStaticEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // コントローラー対応用
    controlTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/right_stick.png");
    // キーボード対応用
    keyBoardTex = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tutorial/left_click_press_release.png");

    // チュートリアル画像の作成
    //「左クリック長押しで 方向をきめよう！」　右スティックを傾けて方向を決めよう！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/left_click_press_release.png", "left_click_press_release");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
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
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;
    startDash = false; // 歩き始めたかどうか

    // 敵を生成する
    owner->ReserveSpawnEnemy({ 12,0,12 }, YarnEnemyType::Static, false, 0.0f, { 0,0,0 }, false, true);
    owner->ReserveSpawnEnemy({ 15,0,12 }, YarnEnemyType::Static, false, 0.0f, { 0,0,0 }, false, true);
    owner->ReserveSpawnEnemy({ 9,0,12 }, YarnEnemyType::Static, false, 0.0f, { 0,0,0 }, false, true);
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

    if (InputSystem::IsGamepadConnected())
    {//　コントローラー対応
        tutorialImage->SetTexture(controlTex);
    }
    else
    {
        tutorialImage->SetTexture(keyBoardTex);
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_SpawnStaticEnemy::Exit()
{
    tutorialImage->SetVisible(false);
}

// ------------------------------ TutorialStep_TiedEnemy ------------------------------
  //「敵をぬいとめたよ！」「ぬいとめた敵に もう一度ぬいダッシュ！」
  //// コンストラクタ
TutorialStep_TiedEnemy::TutorialStep_TiedEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();
    // チュートリアル画像の作成
    //「敵をぬいとめたよ！」「ぬいとめた敵に もう一度ぬいダッシュ！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/tied_enemy_1.png", "tied_enemy_1");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
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

    if (owner->HasDeadEnemy())
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_NiceAttackEnemy");
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_TiedEnemy::Exit()
{
    // チュートリアル画像の切り替え
    tutorialImage->SetVisible(false);

    // チュートリアルターゲットから外す
    owner->ClearTutorialTargets();
}


// ------------------------------ TutorialStep_NiceAttackEnemy ------------------------------
// 「いいね！敵は時間がたつと ぬいとめがほどけちゃうよ！」
// コンストラクタ
TutorialStep_NiceAttackEnemy::TutorialStep_NiceAttackEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //「敵をぬいとめたよ！」「ぬいとめた敵に もう一度ぬいダッシュ！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/nice_attack_enemy.png", "nice_attack_enemy");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_NiceAttackEnemy::~TutorialStep_NiceAttackEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }

}

void TutorialStep_NiceAttackEnemy::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);

}

// ステートで実行するメソッド
void TutorialStep_NiceAttackEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_SpawnMoveEnemy");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_NiceAttackEnemy::Exit()
{
    // チュートリアル画像の切り替え
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_SpawnMoveEnemy ------------------------------
//　「次は動く敵をぬいとめよう！」
// コンストラクタ
TutorialStep_SpawnMoveEnemy::TutorialStep_SpawnMoveEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　「次は動く敵をぬいとめよう！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/next_move_enemy.png", "next_move_enemy");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_SpawnMoveEnemy::~TutorialStep_SpawnMoveEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_SpawnMoveEnemy::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    // 敵を生成する
    owner->ReserveSpawnEnemy({ 11,0,21 }, YarnEnemyType::MoveLinear, false, 2.0f, { 0,0,-1 }, false, true);
    owner->ReserveSpawnEnemy({ 13,0,21 }, YarnEnemyType::MoveLinear, false, 2.0f, { 0,0,-1 }, false, true);
}

// ステートで実行するメソッド
void TutorialStep_SpawnMoveEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    if (owner->HasPinnedEnemy())
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_TiedMoveEnemy");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_SpawnMoveEnemy::Exit()
{
    tutorialImage->SetVisible(false);
}



// ------------------------------ TutorialStep_TiedMoveEnemy ------------------------------
// 「ぬいとめると 動きが止まるよ！そのまま もう一度ぬいダッシュ！」
// コンストラクタ
TutorialStep_TiedMoveEnemy::TutorialStep_TiedMoveEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　「次は動く敵をぬいとめよう！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/enemy_move_stop.png", "enemy_move_stop");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_TiedMoveEnemy::~TutorialStep_TiedMoveEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_TiedMoveEnemy::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

}

// ステートで実行するメソッド
void TutorialStep_TiedMoveEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    if (owner->HasDeadEnemy())
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_DecreaseHeart");
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_TiedMoveEnemy::Exit()
{
    tutorialImage->SetVisible(false);

    owner->ClearTutorialTargets();
}

// ------------------------------ TutorialStep_DecreaseHeart ------------------------------
// 「ぬいダッシュしていない時に敵にぶつかると ハートが減っちゃうよ！」
// コンストラクタ
TutorialStep_DecreaseHeart::TutorialStep_DecreaseHeart(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　ぬいダッシュしていない時に敵にぶつかると ハートが減っちゃうよ！」
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/decrease_heart.png", "decrease_heart");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_DecreaseHeart::~TutorialStep_DecreaseHeart()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_DecreaseHeart::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);

}

// ステートで実行するメソッド
void TutorialStep_DecreaseHeart::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;


    UpdateMouseClickBlink(deltaTime);


    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_DashClothSide");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_DecreaseHeart::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_DashClothSide ------------------------------
// 今度は布の端に向かってぬいダッシュ！
// コンストラクタ
TutorialStep_DashClothSide::TutorialStep_DashClothSide(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    // 今度は布の端に向かってぬいダッシュ！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/next_dash_cloth_side.png", "next_dash_cloth_side");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_DashClothSide::~TutorialStep_DashClothSide()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_DashClothSide::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    // 矢印を出す
    owner->ShowArrows();
}

// ステートで実行するメソッド
void TutorialStep_DashClothSide::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    if (auto player = owner->GetPlayer())
    {
        if (isUseRedirect)
        {//　反射したら
            owner->GetTutorialManager()->ChangeState("TutorialStep_DashRedirect");
        }
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_DashClothSide::Exit()
{
    tutorialImage->SetVisible(false);

    // 矢印を消す
    owner->HideArrows();
}

// ------------------------------ TutorialStep_DashRedirect ------------------------------
// 布の端で”ぬい返り”するよ！
// コンストラクタ
TutorialStep_DashRedirect::TutorialStep_DashRedirect(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　布の端で”ぬい返り”するよ！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/dash_redirect.png", "dash_redirect");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_DashRedirect::~TutorialStep_DashRedirect()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_DashRedirect::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_DashRedirect::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_AttackEnemyRedirect");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_DashRedirect::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}

// ------------------------------ TutorialStep_AttackEnemyRedirect ------------------------------
// ぬい返りで敵を倒してみよう！
// コンストラクタ
TutorialStep_AttackEnemyRedirect::TutorialStep_AttackEnemyRedirect(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　ぬい返りで敵を倒してみよう！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/attack_enemy_redirect.png", "attack_enemy_redirect");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_AttackEnemyRedirect::~TutorialStep_AttackEnemyRedirect()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_AttackEnemyRedirect::Enter()
{
    tutorialImage->SetVisible(true);
    elapsedTime = 0.0f;

    SpawnRedirectEnemies();

    waitingRespawn = false;
}

// ステートで実行するメソッド
void TutorialStep_AttackEnemyRedirect::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

#if 1
    // 敵が全滅したら
    if (owner->GetEnemyCount() <= 0)
    {
        // まだ補充してないなら
        if (!waitingRespawn)
        {
            waitingRespawn = true;

            SpawnRedirectEnemies();
        }
    }
    else
    {
        // 敵が存在するなら次の全滅を待つ
        waitingRespawn = false;
    }
#endif // 0

    if (isRedirectKillEnemy)
    {// 反射で敵を倒したら
        owner->GetTutorialManager()->ChangeState("TutorialStep_RedirectHighScore");
        waitingRespawn = true;
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_AttackEnemyRedirect::Exit()
{
    tutorialImage->SetVisible(false);
}

// 敵がいなくなったら敵を発生させる
void TutorialStep_AttackEnemyRedirect::SpawnRedirectEnemies() const
{
    // 敵を生成する
    owner->ReserveSpawnEnemy({ 17,0,22 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
    owner->ReserveSpawnEnemy({ 19,0,20 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
    owner->ReserveSpawnEnemy({ 21,0,18 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
    owner->ReserveSpawnEnemy({ 3,0,6 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
    owner->ReserveSpawnEnemy({ 5,0,5 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
    owner->ReserveSpawnEnemy({ 7,0,3 }, YarnEnemyType::Static, false, 2.0f, { 0,0,0 }, true, false);
}

// ------------------------------ TutorialStep_RedirectHighScore ------------------------------
//  ぬい返りで倒すと高スコア！
// コンストラクタ
TutorialStep_RedirectHighScore::TutorialStep_RedirectHighScore(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　布の端で”ぬい返り”するよ！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/redirect_high_score.png", "redirect_high_score");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_RedirectHighScore::~TutorialStep_RedirectHighScore()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_RedirectHighScore::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_RedirectHighScore::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    UpdateMouseClickBlink(deltaTime);


    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_AttackAllEnemy");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_RedirectHighScore::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}


// ------------------------------ TutorialStep_AttackAllEnemy ------------------------------
// まとめて敵を倒してみよう
// コンストラクタ
TutorialStep_AttackAllEnemy::TutorialStep_AttackAllEnemy(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　まとめて敵を倒してみよう
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/next_attack_all.png", "next_attack_all");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_AttackAllEnemy::~TutorialStep_AttackAllEnemy()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_AttackAllEnemy::Enter()
{
    tutorialImage->SetVisible(true);
    elapsedTime = 0.0f;

    // 敵を生成する
    spawnPositions =
    {
        {8,0,11},
        {9,0,10},
        {10,0,11},
        {11,0,10},
        {12,0,11},

        {13,0,15},
        {14,0,14},
        {15,0,15},
        {16,0,14},
        {17,0,15},
    };

    for (auto& pos : spawnPositions)
    {
        SpawnEnemyAt(pos);
    }

    waitSpawn = false;
}

// ステートで実行するメソッド
void TutorialStep_AttackAllEnemy::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    constexpr int targetEnemyCount = 7;

    int currentCount = owner->GetAliveEnemyCount();


    if (currentCount <= targetEnemyCount&&!waitSpawn)
    {
        int needSpawn = targetEnemyCount - currentCount;

        for (int i = 0; i < needSpawn; i++)
        {
            XMFLOAT3 pos;

            if (FindEmptySpawnPosition(pos))
            {
                SpawnEnemyAt(pos);
            }
        }
    }

    if (isBonusKill5Enemy)
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_AttackAllBonus");
        waitSpawn = true;
    }

}

// ステージから出ていくときのメソッド
void TutorialStep_AttackAllEnemy::Exit()
{
    tutorialImage->SetVisible(false);
}

// 敵を出現させる
void TutorialStep_AttackAllEnemy::SpawnEnemyAt(const DirectX::XMFLOAT3& pos)
{
    owner->ReserveSpawnEnemy(
        pos,
        YarnEnemyType::Static,
        false,
        2.0f,
        { 0,0,0 },
        true,
        true,false);
}

// 敵と指定した位置が近いかどうか
bool TutorialStep_AttackAllEnemy::IsEnemyNearPosition(
    const XMFLOAT3& pos,
    float radius)
{
    for (auto& target : owner->GetTutorialEnemies())
    {
        auto enemy = target.enemy.lock();

        if (!enemy)
            continue;

        if (enemy->IsDead())
            continue;

        float dist =
            MathHelper::Distance(
                enemy->GetPosition(),
                pos);

        if (dist < radius)
        {
            return true;
        }
    }

    return false;
}

// 生成する位置
bool TutorialStep_AttackAllEnemy::FindEmptySpawnPosition(
    XMFLOAT3& outPos)
{
    std::vector<XMFLOAT3> candidates;

    for (auto& pos : spawnPositions)
    {
        if (!IsEnemyNearPosition(pos, 2.0f))
        {
            candidates.push_back(pos);
        }
    }

    if (candidates.empty())
    {
        return false;
    }

    int index =
        MathHelper::RandomRange(
            0,
            static_cast<int>(candidates.size()) - 1);

    outPos = candidates[index];

    return true;
}
// ------------------------------ TutorialStep_AttackAllBonus ------------------------------
//  5体以上倒すと高スコア！
// コンストラクタ
TutorialStep_AttackAllBonus::TutorialStep_AttackAllBonus(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　5体以上倒すと高スコア！
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/attack_all_bonus.png", "attack_all_bonus");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_AttackAllBonus::~TutorialStep_AttackAllBonus()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_AttackAllBonus::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_AttackAllBonus::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;
    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        owner->GetTutorialManager()->ChangeState("TutorialStep_StageClear");
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_AttackAllBonus::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}

// ------------------------------ TutorialStep_StageClear ------------------------------
//  ステージクリア条件
// コンストラクタ
TutorialStep_StageClear::TutorialStep_StageClear(TutorialActor* actor) :TutorialStep(actor)
{
    auto uiManager = Scene::GetCurrentScene()->GetUIManager();

    // チュートリアル画像の作成
    //　 ステージクリア条件
    tutorialImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tutorial/stage_clear.png", "stage_clear");
    tutorialImage->SetWorldPosition(imagePos);
    tutorialImage->SetSize(imageSize);
    tutorialImage->SetVisible(false);
    uiManager->Add(tutorialImage);
}

// デストラクタ
TutorialStep_StageClear::~TutorialStep_StageClear()
{
    if (tutorialImage)
    {// 削除通知を出す
        tutorialImage->MarkPendingKill();
    }
}

// ステートに入った時のメソッド
void TutorialStep_StageClear::Enter()
{
    tutorialImage->SetVisible(true);

    elapsedTime = 0.0f;

    ResetMouseClickBlink();
    ShowMouseClick(true);
}

// ステートで実行するメソッド
void TutorialStep_StageClear::Execute(float deltaTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    UpdateMouseClickBlink(deltaTime);

    //  押した瞬間
    if (InputSystem::GetInputState("TutorialOk", InputStateMask::Release))
    {
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("type", types[rand() % 2]) });
        CoreAudio::PlayOneShot(L"./Data/Sound/SE/click_se.wav", 2.0f);
    }
}

// ステージから出ていくときのメソッド
void TutorialStep_StageClear::Exit()
{
    tutorialImage->SetVisible(false);
    ShowMouseClick(false);
}

