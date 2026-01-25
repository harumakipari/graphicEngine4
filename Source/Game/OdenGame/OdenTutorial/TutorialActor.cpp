#include "pch.h"
#include "TutorialActor.h"
#include "TutorialStep.h"
#include "TutorialManager.h"
#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Physics/CollisionFunction.h"

void TutorialActor::Initialize(const Transform& transform)
{
    tutorialManager = std::make_unique<TutorialManager>();
    // 各ステートを登録
    tutorialManager->RegisterState(std::make_unique<TutorialStep_StartOdenStore>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_TakeOdenIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SubmitClearIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ComeOdenCircleIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SubmitOdenCircleIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ClearCircleIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_RotateOdenIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ComeOdenSquareIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SubmitOdenSquareIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ClearSquareIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_SwapIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ClearSwapIngredient>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_IntroduceShape>(this));
    tutorialManager->RegisterState(std::make_unique<TutorialStep_ClearTutorial>(this));


    auto uiManager = Scene::GetCurrentScene()->GetUIManager();


    XMFLOAT2 bubblePos = { -100.0f,-100.0f };
    XMFLOAT2 bubbleSize = { 400.0f,150.0f };


    // チュートリアル画像の作成 
    bubbleUpImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_bubble_up.png", "tutorial_bubble_up");
    bubbleUpImage->SetWorldPosition(bubblePos);
    bubbleUpImage->SetSize(bubbleSize);
    bubbleUpImage->zOrder = 5;
    bubbleUpImage->SetVisible(false);
    uiManager->Add(bubbleUpImage);

    // チュートリアル画像の作成 
    bubbleDownImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_bubble_down.png", "tutorial_bubble_down");
    bubbleDownImage->SetWorldPosition(bubblePos);
    bubbleDownImage->SetSize(bubbleSize);
    bubbleDownImage->zOrder = 5;
    bubbleDownImage->SetVisible(false);
    uiManager->Add(bubbleDownImage);

    // チュートリアル画像の作成 
    bubbleLeftImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_bubble_left.png", "tutorial_bubble_left");
    bubbleLeftImage->SetWorldPosition(bubblePos);
    bubbleLeftImage->SetSize(bubbleSize);
    bubbleLeftImage->zOrder = 5;
    bubbleLeftImage->SetVisible(false);
    uiManager->Add(bubbleLeftImage);

    // チュートリアル画像の作成 
    bubbleRightImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_bubble_right.png", "tutorial_bubble_right");
    bubbleRightImage->SetWorldPosition(bubblePos);
    bubbleRightImage->SetSize(bubbleSize);
    bubbleRightImage->zOrder = 5;
    bubbleRightImage->SetVisible(false);
    uiManager->Add(bubbleRightImage);


    XMFLOAT2 textPos = { -100.0f,-100.0f };
    XMFLOAT2 textSize = { 315.0f,140.0f };

    // チュートリアル画像の作成 
    thisKobumusubiImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_this_kobumusubi.png", "tutorial_this_kobumusubi");
    thisKobumusubiImage->SetWorldPosition(textPos);
    thisKobumusubiImage->SetSize(textSize);
    thisKobumusubiImage->zOrder = 7;
    thisKobumusubiImage->SetVisible(false);
    uiManager->Add(thisKobumusubiImage);

    // チュートリアル画像の作成 
    thisKonnyakuImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_this_konnyaku.png", "tutorial_this_konnyaku");
    thisKonnyakuImage->SetWorldPosition(textPos);
    thisKonnyakuImage->SetSize(textSize);
    thisKonnyakuImage->zOrder = 7;
    thisKonnyakuImage->SetVisible(false);
    uiManager->Add(thisKonnyakuImage);

    // チュートリアル画像の作成 
    thisEggImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_this_egg.png", "tutorial_this_egg");
    thisEggImage->SetWorldPosition(textPos);
    thisEggImage->SetSize(textSize);
    thisEggImage->zOrder = 7;
    thisEggImage->SetVisible(false);
    uiManager->Add(thisEggImage);

    // チュートリアル画像の作成 
    thisGobotenImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_this_goboten.png", "tutorial_this_goboten");
    thisGobotenImage->SetWorldPosition(textPos);
    thisGobotenImage->SetSize(textSize);
    thisGobotenImage->zOrder = 7;
    thisGobotenImage->SetVisible(false);
    uiManager->Add(thisGobotenImage);

    // チュートリアル画像の作成 
    thisChikuwaImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_this_chikuwa.png", "tutorial_this_chikuwa");
    thisChikuwaImage->SetWorldPosition(textPos);
    thisChikuwaImage->SetSize(textSize);
    thisChikuwaImage->zOrder = 7;
    thisChikuwaImage->SetVisible(false);
    uiManager->Add(thisChikuwaImage);

    // チュートリアル画像の作成 
    thisNotCircleImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_not_circle.png", "tutorial_not_circle");
    thisNotCircleImage->SetWorldPosition(textPos);
    thisNotCircleImage->SetSize(textSize);
    thisNotCircleImage->zOrder = 7;
    thisNotCircleImage->SetVisible(false);
    uiManager->Add(thisNotCircleImage);

    // チュートリアル画像の作成 
    thisNotSquareImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Tutorial/tutorial_not_square.png", "tutorial_not_square");
    thisNotSquareImage->SetWorldPosition(textPos);
    thisNotSquareImage->SetSize(textSize);
    thisNotSquareImage->zOrder = 7;
    thisNotSquareImage->SetVisible(false);
    uiManager->Add(thisNotSquareImage);

}

void TutorialActor::Update(float deltaTime)
{
    // チュートリアルマネージャーの更新
    if (tutorialManager)
    {
        tutorialManager->Update(deltaTime);
    }

    if (!isBubbleAutoHide)
        return;

    bubbleLifeTimer += deltaTime;
    if (bubbleLifeTimer >= bubbleLifeTime)
    {
        HideAllBubbles();
        isBubbleAutoHide = false;
    }
}

void TutorialActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat2("ui_offset", &uiOffsetPos.x);
    ImGui::DragFloat2("ui_text_offset", &uiTextOffsetPos.x);
#endif
}

// チュートリアル開始処理
void TutorialActor::StartTutorial()
{
    // 最初のステートに変更
    tutorialManager->ChangeState("StartOdenStore");
}

void TutorialActor::OnIngredientGrabbed(const std::shared_ptr<Actor>& ingredient)
{
    grabbedIngredient = std::dynamic_pointer_cast<OdenIngredientActor>(ingredient);
}

// 今掴まれている食材を伝える関数
std::shared_ptr<OdenIngredientActor> TutorialActor::GetGrabbedIngredient() const
{
    return grabbedIngredient.lock();
}

// ステップ中に掴まれている食材をリセットする
void TutorialActor::ClearGrabbedIngredient()
{
    grabbedIngredient.reset();
}
// 吹き出しを表示する
void TutorialActor::ShowBalloonNearIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient)
{
    HideAllBubbles();

    DirectX::XMFLOAT3 worldPos = ingredient->GetPosition();
    //worldPos.y += 2.0f; // 少し上

    DirectX::XMFLOAT2 uiPos = WorldToUI(worldPos);
    uiPos.x += uiOffsetPos.x;
    uiPos.y += uiOffsetPos.y;
    DirectX::XMFLOAT2 textPos = WorldToUI(worldPos);
    textPos.x += uiTextOffsetPos.x;
    textPos.y += uiTextOffsetPos.y;
    {
        bubbleLeftImage->SetWorldPosition(uiPos);
        bubbleLeftImage->SetVisible(true);
    }

    // 食材の種類
    EOdenType type = ingredient->GetIngredientType();
    switch (type)
    {
    case EOdenType::Egg:
        thisEggImage->SetWorldPosition(textPos);
        thisEggImage->SetVisible(true);
        break;
    case EOdenType::Chikuwa:
        thisChikuwaImage->SetWorldPosition(textPos);
        thisChikuwaImage->SetVisible(true);
        break;
    case EOdenType::Konnyaku:
        thisKonnyakuImage->SetWorldPosition(textPos);
        thisKonnyakuImage->SetVisible(true);
        break;
    case EOdenType::Goboten:
        thisGobotenImage->SetWorldPosition(textPos);
        thisGobotenImage->SetVisible(true);
        break;
    case EOdenType::Kobumusubi:
        thisKobumusubiImage->SetWorldPosition(textPos);
        thisKobumusubiImage->SetVisible(true);
        break;

    }
    // 何秒後に非表示設定をする
    SetBubbleTime();
}

// 吹き出しを表示する　丸っぽいのチュートリアル用
void TutorialActor::ShowCircleBallonNearIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient)
{
    HideAllBubbles();

    DirectX::XMFLOAT3 worldPos = ingredient->GetPosition();
    //worldPos.y += 2.0f; // 少し上

    DirectX::XMFLOAT2 uiPos = WorldToUI(worldPos);
    uiPos.x += uiOffsetPos.x;
    uiPos.y += uiOffsetPos.y;
    DirectX::XMFLOAT2 textPos = WorldToUI(worldPos);
    textPos.x += uiTextOffsetPos.x;
    textPos.y += uiTextOffsetPos.y;
    {
        bubbleLeftImage->SetWorldPosition(uiPos);
        bubbleLeftImage->SetVisible(true);
    }

    // 面の種類
    EOdenShapeCategory type = ingredient->GetCurrentShape().category;
    switch (type)
    {
    case EOdenShapeCategory::TriangleLike:
        thisNotCircleImage->SetWorldPosition(textPos);
        thisNotCircleImage->SetVisible(true);
        break;
    case EOdenShapeCategory::SquareLike:
        thisNotCircleImage->SetWorldPosition(textPos);
        thisNotCircleImage->SetVisible(true);
        break;
    case EOdenShapeCategory::RibbonLike:
        thisNotCircleImage->SetWorldPosition(textPos);
        thisNotCircleImage->SetVisible(true);
        break;
    case EOdenShapeCategory::DonutLike:
        thisNotCircleImage->SetWorldPosition(textPos);
        thisNotCircleImage->SetVisible(true);
        break;
    }
    // 何秒後に非表示設定をする
    SetBubbleTime();

}

// 吹き出しを表示する　四角っぽいのチュートリアル用
void TutorialActor::ShowSquareBallonNearIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient)
{
    HideAllBubbles();

    DirectX::XMFLOAT3 worldPos = ingredient->GetPosition();
    //worldPos.y += 2.0f; // 少し上

    DirectX::XMFLOAT2 uiPos = WorldToUI(worldPos);
    uiPos.x += uiOffsetPos.x;
    uiPos.y += uiOffsetPos.y;
    DirectX::XMFLOAT2 textPos = WorldToUI(worldPos);
    textPos.x += uiTextOffsetPos.x;
    textPos.y += uiTextOffsetPos.y;
    {
        bubbleLeftImage->SetWorldPosition(uiPos);
        bubbleLeftImage->SetVisible(true);
    }

    // 面の種類
    EOdenShapeCategory type = ingredient->GetCurrentShape().category;
    switch (type)
    {
    case EOdenShapeCategory::TriangleLike:
        thisNotSquareImage->SetWorldPosition(textPos);
        thisNotSquareImage->SetVisible(true);
        break;
    case EOdenShapeCategory::RibbonLike:
        thisNotSquareImage->SetWorldPosition(textPos);
        thisNotSquareImage->SetVisible(true);
        break;
    case EOdenShapeCategory::DonutLike:
        thisNotSquareImage->SetWorldPosition(textPos);
        thisNotSquareImage->SetVisible(true);
        break;
    case EOdenShapeCategory::RoundLike:
        thisNotSquareImage->SetWorldPosition(textPos);
        thisNotSquareImage->SetVisible(true);
        break;
    }
    // 何秒後に非表示設定をする
    SetBubbleTime();

}

// 吹き出しや文字を全て非表示にする関数
void TutorialActor::HideAllBubbles()
{
    bubbleUpImage->SetVisible(false);
    bubbleDownImage->SetVisible(false);
    bubbleLeftImage->SetVisible(false);
    bubbleRightImage->SetVisible(false);

    thisEggImage->SetVisible(false);
    thisChikuwaImage->SetVisible(false);
    thisKonnyakuImage->SetVisible(false);
    thisGobotenImage->SetVisible(false);
    thisKobumusubiImage->SetVisible(false);

    thisNotCircleImage->SetVisible(false);
    thisNotSquareImage->SetVisible(false);
}

// 何秒後に非表示設定をする
void TutorialActor::SetBubbleTime()
{
    bubbleLifeTime = 1.5f;
    bubbleLifeTimer = 0.0f;
    isBubbleAutoHide = true;
    CoreAudio::PlayOneShot(L"./Data/Sound/SE/mistake_click.wav", 1.0f);
}