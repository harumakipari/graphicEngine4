#include "pch.h"
#include "BookBaseActor.h"

#include <magic_enum.hpp>

#include "SaveDataManager.h"
#include "ScoreCalculator.h"
#include "TitleScene.h"
#include "Engine/Audio/CoreAudio.h"
#include "Physics/CollisionFunction.h"

void BookBaseActor::Initialize(const Transform& transform)
{
    parentName = "BookBaseActor";
    //矢印UIを表示する
    showSecondPageButtonArrow = true;
}

void BookBaseActor::Update(float deltaTime)
{
    // コントローラー対応処理
    HandlePadInput();

    easingOneRunner->Tick(deltaTime);
    easingTwoRunner->Tick(deltaTime);

    // ロック中のワッペンの彩度を下げる
    for (auto& stage : leftPage.stages)
    {
        UpdateStageVisual(stage);
    }
    for (auto& stage : rightPage.stages)
    {
        UpdateStageVisual(stage);
    }

    if (!InputSystem::IsGamepadConnected())
    {
        startAButton->SetVisible(false);
    }

    switch (bookState)
    {
    case BookPageState::Closed:
        UpdateClosedBook();
        break;
    case BookPageState::FirstPage:
        UpdatePage(leftPage);
        UpdatePage(rightPage);
        HandlePadStageSelection(deltaTime);
        break;
    case BookPageState::SecondPage:
        break;
    case BookPageState::OpeningBook:
        bookSpineModel->SetRelativeScaleDirect({ 1.0f,1.0f,1.0f });
        //if (isOpeningBook && bookOneAlpha > 0.8f)
        //{
        //    bookState = BookPageState::FirstPage;
        //    isOpeningBook = false;
        //}

        break;
    case BookPageState::OpeningSecondPage:
        //if (bookTwoAlpha > 0.8f)
        //{
        //    bookState = BookPageState::SecondPage;
        //}
        break;
    case BookPageState::ClosingBook:
        //if (isClosingBook && bookOneAlpha < 0.2f)
        //{
        //    bookState = BookPageState::Closed;
        //    isClosingBook = false;
        //}

        break;
    case BookPageState::ReturningFirstPage:
        //if (bookTwoAlpha < 0.2f)
        //{
        //    bookState = BookPageState::FirstPage;
        //}
        break;
    }

    Logger::Log(std::string(magic_enum::enum_name(bookState)));

    bool isClosing = (bookState == BookPageState::ClosingBook || bookState == BookPageState::Closed);

    if (!isClosing)
    {
        if (bookState != BookPageState::FirstPage)
        {
            if (startAButton)
            {
                startAButton->SetVisible(false);
            }
        }

        // 矢印UIの表示非表示
        firstButtons.SetEnable(bookState == BookPageState::FirstPage);
        if (showSecondPageButtonArrow)
        {
            secondButtons.SetEnable(bookState == BookPageState::SecondPage);
        }
        else
        {
            secondButtons.SetEnable(false);
        }
        // 矢印UIのコントローラー接続によるテクスチャの切り替え
        firstButtons.UpdateInputTexture();
        secondButtons.UpdateInputTexture();
    }
    else
    {
        // ← ClosingBook中は強制非表示
        firstButtons.SetEnable(false);
        secondButtons.SetEnable(false);

        if (startAButton)
        {
            startAButton->SetVisible(false);
        }
    }

    // 一ページ目の処理
    {
        // 本
        float bookAngle = std::lerp(closeBookAngle, openBookAngle, bookOneAlpha);
        bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,bookAngle });
        // 背表紙角度
        float spineAngle = std::lerp(openSpineEuler, closeSpineEuler, bookOneAlpha);
        bookSpineModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,spineAngle });
        // 背表紙位置
        float spinePosY = std::lerp(openSpinPosY, closeSpinPosY, bookOneAlpha);
        bookSpineModel->SetRelativeLocationDirect({ 0.1f,spinePosY,0.0f });
    }

    // 二ページ目の処理
    {
        // 本
        float middleAngle = std::lerp(openFirstPageAngle, closeFirstPageAngle, bookTwoAlpha);
        bookMiddleModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,middleAngle });
        for (auto& stage : leftPage.stages)
        {// 左ページのワッペン
            // ワッペンの位置を下げる
            float patchOffsetY = std::lerp(openPatchPosY, closePatchPosY, bookTwoAlpha);
            stage->model->SetRelativeLocationDirect({ stage->offsetPos.x,patchOffsetY, stage->offsetPos.z });
        }
    }
}

void BookBaseActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("本が開く")))
    {
        OpenBook(2.0f);
    }
    if (ImGui::Button(U8("一ページ目をめくる")))
    {
        OpenSecondPage(2.0f);
    }
    if (ImGui::Button(U8("一ページ目を戻す")))
    {
        CloseSecondPage(2.0f);
    }
    if (ImGui::Button(U8("本が閉じる")))
    {
        CloseBook(2.0f);
    }
    ImGui::DragFloat3(U8("ワッペンのオフセット"), &patchAButtonOffset.x);
    ImGui::DragFloat(U8("ワッペンのスケール"), &patchBaseScale, 0.05f, 1.0f, 1.5f);
    ImGui::DragFloat(U8("ワッペンのホバースケール"), &patchHoverScale, 0.05f, 1.0f, 1.5f);

#endif
}

// 本を開く
void BookBaseActor::OpenBook(float interval, bool playSe, std::function<void()> completed)
{
    // 本を開ける音
    if (playSe)
    {
        CoreAudio::PlayOneShot(L"./Data/Sound/SE1/open_book.wav", 1.5f);
    }

    isOpeningBook = true;
    isClosingBook = false;

    bookState = BookPageState::OpeningBook;
    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this, completed]()
        {
            bookOneAlpha = 1.0f;
            bookState = BookPageState::FirstPage;
            if (completed)
            {
                completed();
            }

        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return bookOneAlpha;
        };

    accessor.setter =
        [this](float t)
        {
            bookOneAlpha = t;
        };

    easingOneRunner->StartHandler(handler, accessor);
}

// 本を閉じる
void BookBaseActor::CloseBook(float interval)
{
    // 本を閉じる音
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/close_book.wav", 1.5f);

    isOpeningBook = false;
    isClosingBook = true;

    bookState = BookPageState::ClosingBook;
    if (bookTwoAlpha > 0.001f)
    {// 本が二枚目の時に

        CloseSecondPage(interval * firstRate);
        TestEasingHandler waitHandler;
        waitHandler.AddWait(interval * firstRate);

        waitHandler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            0.0f,
            interval * secondRate
        );

        waitHandler.SetCompletedFunction([this]()
            {
                bookState = BookPageState::Closed;
                bookOneAlpha = 0.0f;
            });

        PropertyAccessor<float> accessor;

        accessor.getter =
            [this]()
            {
                return bookOneAlpha;
            };

        accessor.setter =
            [this](float t)
            {
                bookOneAlpha = t;
            };

        easingOneRunner->StartHandler(waitHandler, accessor);

        return;
    }

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        1.0f,
        0.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            bookOneAlpha = 0.0f;
            bookState = BookPageState::Closed;
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return bookOneAlpha;
        };

    accessor.setter =
        [this](float t)
        {
            bookOneAlpha = t;
        };

    easingOneRunner->StartHandler(handler, accessor);
}

// 二ページ目を開く
void BookBaseActor::OpenSecondPage(float interval, std::function<void()> completed)
{
    // ページをめくる音
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/page.wav", 1.5f);

    bookState = BookPageState::OpeningSecondPage;
    easingTwoRunner->Clear();

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this, completed]()
        {
            bookTwoAlpha = 1.0f;
            bookState = BookPageState::SecondPage;
            if (completed)
            {
                completed();
            }
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return bookTwoAlpha;
        };

    accessor.setter =
        [this](float t)
        {
            bookTwoAlpha = t;
        };

    easingTwoRunner->StartHandler(handler, accessor);
}

// 二ページ目を戻す処理
void BookBaseActor::CloseSecondPage(float interval)
{
    // ページをめくる音
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/page.wav", 1.5f);

    bookState = BookPageState::ReturningFirstPage;

    easingTwoRunner->Clear();

    TestEasingHandler handler;

    handler.AddEasing(
        TestEaseType::OutExp,
        bookTwoAlpha,
        0.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            bookTwoAlpha = 0.0f;
            bookState = BookPageState::FirstPage;
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return bookTwoAlpha;
        };

    accessor.setter =
        [this](float t)
        {
            bookTwoAlpha = t;
        };

    easingTwoRunner->StartHandler(handler, accessor);
}

// ステージの開放状態を更新する
void BookBaseActor::RefreshStageUnlock()
{
    selectableStages.clear();

    for (auto& stage : leftPage.stages)
    {
        stage->isUnlocked =
            SaveDataManager::Instance().IsStageUnlocked(stage->stage);

        if (stage->isUnlocked)
        {
            selectableStages.push_back(stage);
        }
    }
    for (auto& stage : rightPage.stages)
    {
        stage->isUnlocked =
            SaveDataManager::Instance().IsStageUnlocked(stage->stage);

        if (stage->isUnlocked)
        {
            selectableStages.push_back(stage);
        }
    }
}

// 最初の本の状態を設定する
void BookBaseActor::SetInitPageState(BookPageState initialState)
{
    bookState = initialState;
    if (initialState == BookPageState::Closed)
    {
        bookOneAlpha = 0.0f;
        bookTwoAlpha = 0.0f;
    }
    else if (initialState == BookPageState::FirstPage)
    {
        bookOneAlpha = 1.0f;
        bookTwoAlpha = 0.0f;
    }
    else if (initialState == BookPageState::SecondPage)
    {
        bookOneAlpha = 1.0f;
        bookTwoAlpha = 1.0f;
    }
}

// モデルを生成する
void BookBaseActor::CreateBookModel(const std::string& backCoverModelName, const std::string& middleModelName)
{
    // 裏表紙のモデル名
    this->backCoverModelName = backCoverModelName;
    // 真ん中のモデル名
    this->middleModelName = middleModelName;


    auto rootComponent = AddComponent<SceneComponent>(parentName);

    // 裏表紙を追加
    backCoverName = "bookRightModel";
    bookRightModel = AddComponent<SkeletalMeshComponent>(backCoverName, parentName);
    bookRightModel->SetModel(backCoverModelName, false, false);

    // 表紙モデルを追加
    bookLeftModel = AddComponent<SkeletalMeshComponent>("bookLeftModel", parentName);
    bookLeftModel->SetModel("./Data/TeamModels/Title/BookLeft.gltf", false, false);
    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookLeftModel->SetIsCastShadow(false);

    // 真ん中モデルを追加
    bookMiddleModel = AddComponent<SkeletalMeshComponent>("bookMiddleModel", parentName);
    bookMiddleModel->SetModel(middleModelName, false, false);
    bookMiddleModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookMiddleModel->SetIsCastShadow(false);

    // 背表紙モデル
    bookSpineModel = AddComponent<SkeletalMeshComponent>("bookSpineModel", parentName);
    bookSpineModel->SetModel("./Data/TeamModels/Title/BookSpineModel.gltf", false, false);
    bookSpineModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookSpineModel->SetRelativeLocationDirect({ 0.1f,0.0f,0.0f });
    bookSpineModel->SetIsCastShadow(false);

    bookSpineCollisionComponent = AddComponent<BoxComponent>("bookSpineCollider", "bookSpineModel");
    DirectX::XMFLOAT3 size = bookSpineModel->GetModelSize();
    bookSpineCollisionComponent->SetBoxExtent(size);
    bookSpineCollisionComponent->SetLayer(CollisionLayer::WorldStatic);
    bookSpineCollisionComponent->Initialize();

    easingOneRunner = std::make_unique<EasingRunner>();
    easingTwoRunner = std::make_unique<EasingRunner>();

    bookOneAlpha = 0.0f;
    bookTwoAlpha = 0.0f;

#if 1
    // 左の本モデル
    leftPage.parentName = "bookLeftModel";
    // 一枚目の開く時の右の本モデル
    rightPage.parentName = "bookMiddleModel";    // 真ん中のページ

    CreateStagePatch(
        leftPage,
        STAGE_NAME::TUTORIAL,
        "./Data/TeamModels/Title/patchModelTutorial.gltf",
        { 3.3f,0.0f,-1.5f });

    CreateStagePatch(
        leftPage,
        STAGE_NAME::FIRST,
        "./Data/TeamModels/Title/patchModelFirst.gltf",
        { 1.3f,0.0f,-0.1f });

    CreateStagePatch(
        leftPage,
        STAGE_NAME::BOBBIN_FIRST,
        "./Data/TeamModels/Title/patchModelBobbinFirst.gltf",
        { 2.9f,0.0f,1.7f });

    CreateStagePatch(
        rightPage,
        STAGE_NAME::REFLECT_WALL,
        "./Data/TeamModels/Title/patchModelReflectWall.gltf",
        { -3.2f,0.1f,-1.6f });

    CreateStagePatch(
        rightPage,
        STAGE_NAME::DIFFICULT,
        "./Data/TeamModels/Title/patchModelDifficult.gltf",
        { -1.3f,0.1f,-0.1f });


    CreateStagePatch(
        rightPage,
        STAGE_NAME::BOSS,
        "./Data/TeamModels/Title/patchModelBoss.gltf",
        { -3.4f,0.1f,1.7f });

#endif // 0

    int clearIndex = static_cast<int>(ScoreSystem::GetResultStats().stageName);

    if (clearIndex > 0)
    {
        selectedStageIndex = clearIndex;
    }
    else
    {
        selectedStageIndex = 0;
    }

    // AボタンのUIを生成する
    auto uiManager = GetOwnerScene()->GetUIManager();
    startAButton = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/A.png", "A");
    startAButton->SetWorldPosition({ 880, 920 });
    startAButton->SetSize({ 80, 80 });
    startAButton->SetPivot({ 0.5f,0.5f });
    startAButton->SetVisible(false);
    uiManager->Add(startAButton);
}

// ステージパッチを生成する
void BookBaseActor::CreateStagePatch(BookPage& page, STAGE_NAME stage, const char* modelPath, const DirectX::XMFLOAT3& pos)
{
    std::string name =
        "patch_" + std::to_string(static_cast<int>(stage));

    std::string boxName =
        "patch_box_" + std::to_string(static_cast<int>(stage));

    auto model =
        AddComponent<SkeletalMeshComponent>(
            name,
            page.parentName);

    model->SetModel(modelPath, false, false);
    model->SetRelativeLocationDirect(pos);
    model->SetIsCastShadow(false);
    model->overrideDeferredPipelineName = "GameModelColorFilterPS";
    model->SetRelativeScaleDirect({ 1.1f,1.1f,1.1f });

    auto box =
        AddComponent<BoxComponent>(
            boxName,
            name);

    DirectX::XMFLOAT3 size = model->GetModelSize();
    box->SetBoxExtent(size);
    box->SetLayer(CollisionLayer::WorldStatic);
    box->Initialize();

    auto stageData =
        std::make_shared<StageSelectData>();

    stageData->stage = stage;
    stageData->model = model;
    stageData->collider = box;
    stageData->offsetPos = pos;
    stageData->isUnlocked = SaveDataManager::Instance().IsStageUnlocked(stage);

    page.stages.push_back(stageData);

    if (stageData->isUnlocked)
    {
        selectableStages.push_back(stageData);
    }
}

// ページのパッチの更新処理
void BookBaseActor::UpdatePage(const BookPage& page)
{
    DirectX::XMFLOAT2 cursor;

    // マウスが画面外
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    if (InputSystem::IsGamepadConnected())
    {// ゲームパッドが繋がれていたら
        return;
    }

    HitResultWithActor result;

    bool hit =
        CollisionFunction::RaycastFromMouse(
            cursor,
            result,
            CollisionHelper::ToBit(CollisionLayer::WorldStatic));

    for (auto& stage : page.stages)
    {
        bool hitThis =
            hit &&
            result.component == stage->collider.get();


        if (!stage->isUnlocked)
        {
            // ロック中はホバー音なし
            stage->wasLockedHovered = hitThis;

            // クリックしたときだけ鳴らす
            if (hitThis &&
                InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
            {
                CoreAudio::PlayOneShot(L"./Data/Sound/SE1/cancel.wav");
            }
            continue;
        }

        //stage->model->plusAlphaCBuffer->data.saturation = 0.f;// 彩度を戻す



        // ホバー開始を検出
        bool hoverEnter = hitThis && !stage->wasHovered;

        if (hoverEnter)
        {
            // 新しくかざされた時に音を出す
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/stage_select_control.wav");
        }

        stage->wasHovered = hitThis;

        // ホバー演出
        if (hitThis)
        {


            stage->model->SetRelativeScaleDirect(
                {
                    patchHoverScale,
                    patchHoverScale,
                    patchHoverScale
                });
        }
        else
        {
            stage->model->SetRelativeScaleDirect(
                {
                    patchBaseScale,
                    patchBaseScale,
                    patchBaseScale
                });
        }

        // クリック
        if (hitThis &&
            InputSystem::GetInputState(
                "MouseLeft",
                InputStateMask::Trigger))
        {
            Logger::Log(u8"ステージ選択");
            // ステージ決定音
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/push_button.wav");

            if (stage->stage == STAGE_NAME::TUTORIAL)
            {// チュートリアルを選択した時のみ
                // 遊ぶステージ名を記録する
                ScoreSystem::RecordStageName(stage->stage);

                SceneTransitionManager::SceneTransitionParam param;

                param["preload"] = "TutorialScene";
                param["stage"] = std::string(magic_enum::enum_name(stage->stage));
                param["fromScene"] = "StageSelect";

                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    param
                );
#if 0
                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    {
                        {"preload", "TutorialScene"},
                        {
                            "stage",
                            std::string(
                                magic_enum::enum_name(stage->stage))
                        },
                        {"fromScene","StageSelect"},
                    });

#endif // 0
            }
            else
            {
                // 遊ぶステージ名を記録する
                ScoreSystem::RecordStageName(stage->stage);

                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    {
                        {"preload", "GameScene"},
                        {
                            "stage",
                            std::string(
                                magic_enum::enum_name(stage->stage))
                        },
                        {"fromScene","StageSelect"},
                    });

            }
        }
    }
}

// 本を閉じている時の処理
void BookBaseActor::UpdateClosedBook()
{
    DirectX::XMFLOAT2 cursor;

    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    if (InputSystem::IsGamepadConnected())
    {// ゲームパッドが繋がれていたら
        return;
    }

    HitResultWithActor result;

    bool hit = CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic));

    bool hitBook = hit && result.component == bookSpineCollisionComponent.get();

    // ホバー開始
    bool hoverEnter = hitBook && !wasHovered;

    if (hoverEnter)
    {
        // 新しくかざされた時に音を出す
        CoreAudio::PlayOneShot(L"./Data/Sound/SE1/stage_select_control.wav");
    }

    // 状態を保存
    wasHovered = hitBook;

    if (hitBook)
    {
        bookSpineModel->SetRelativeScaleDirect(
            { 1.05f,1.05f,1.05f });
    }
    else
    {
        bookSpineModel->SetRelativeScaleDirect(
            { 1.0f,1.0f,1.0f });
    }

    if (hitBook &&
        InputSystem::GetInputState(
            "MouseLeft",
            InputStateMask::Trigger))
    {
        if (onRequestOpenBook)
        {
            onRequestOpenBook();
        }
    }
}



// コントローラー対応用ステージ選択の処理
void BookBaseActor::HandlePadStageSelection(float deltaTime)
{
    if (!InputSystem::IsGamepadConnected())
    {// ゲームパッドが繋がれていなかったら、
        return;
    }

    static float stickDelay = 0.0f;
    stickDelay -= deltaTime;

    bool moved = false;

    // =========================
    // D-Pad入力（優先・1回だけ）
    // =========================
    if (InputSystem::GetInputState("UIUp", InputStateMask::Trigger))
    {
        MoveSelection(-1);
        stickDelay = 0.2f;
        moved = true;
    }
    else if (InputSystem::GetInputState("UIDown", InputStateMask::Trigger))
    {
        MoveSelection(1);
        stickDelay = 0.2f;
        moved = true;
    }
    else if (InputSystem::GetInputState("UILeft", InputStateMask::Trigger))
    {
        MoveSelection(-3);
        stickDelay = 0.2f;
        moved = true;
    }
    else if (InputSystem::GetInputState("UIRight", InputStateMask::Trigger))
    {
        MoveSelection(3);
        stickDelay = 0.2f;
        moved = true;
    }


    // =========================
    // スティック入力（D-Pad優先）
    // =========================
    if (!moved && stickDelay <= 0.0f)
    {
#if 0
        auto stick = InputSystem::GetLeftStick();

        if (stick.y > 0.6f)
        {
            MoveSelection(-1);
            stickDelay = 0.2f;
        }
        else if (stick.y < -0.6f)
        {
            MoveSelection(1);
            stickDelay = 0.2f;
        }
#else
        auto leftStick = InputSystem::GetLeftStick();
        auto rightStick = InputSystem::GetRightStick();

        auto handleStick = [&](const auto& stick)
            {
                // 縦移動（±1）
                if (stick.y > 0.6f)
                {
                    MoveSelection(-1);
                    stickDelay = 0.2f;
                    return true;
                }
                else if (stick.y < -0.6f)
                {
                    MoveSelection(1);
                    stickDelay = 0.2f;
                    return true;
                }

                // 横移動（±3）
                if (stick.x > 0.6f)
                {
                    MoveSelection(3);
                    stickDelay = 0.2f;
                    return true;
                }
                else if (stick.x < -0.6f)
                {
                    MoveSelection(-3);
                    stickDelay = 0.2f;
                    return true;
                }

                return false;
            };

        // 左スティック優先 → 右スティック
        if (!handleStick(leftStick))
        {
            handleStick(rightStick);
        }

#endif // 0
    }

    for (auto& stage : leftPage.stages)
    {
        bool selectedByPad =
            !selectableStages.empty() &&
            selectableStages[selectedStageIndex] == stage;

        if (selectedByPad)
        {
            stage->model->SetRelativeScaleDirect(
                {
                    patchHoverScale,
                    patchHoverScale,
                    patchHoverScale
                });
        }
        else
        {
            stage->model->SetRelativeScaleDirect(
                {
                    patchBaseScale,
                    patchBaseScale,
                    patchBaseScale
                });
        }

    }
    for (auto& stage : rightPage.stages)
    {
        bool selectedByPad =
            !selectableStages.empty() &&
            selectableStages[selectedStageIndex] == stage;

        if (selectedByPad)
        {
            stage->model->SetRelativeScaleDirect(
                {
                    patchHoverScale,
                    patchHoverScale,
                    patchHoverScale
                });
        }
        else
        {
            stage->model->SetRelativeScaleDirect(
                {
                    patchBaseScale,
                    patchBaseScale,
                    patchBaseScale
                });
        }
    }

#if 1
    // =========================
    // AボタンUI更新
    // =========================

    if (InputSystem::IsGamepadConnected() && !selectableStages.empty())
    {
        auto selectedStage = selectableStages[selectedStageIndex];

        startAButton->SetVisible(true);

        // ワッペン位置取得
        auto world = selectedStage->model->GetComponentLocation();


        DirectX::XMFLOAT3 pos =
        {
            world.x + patchAButtonOffset.x,
            world.y + patchAButtonOffset.y,
            world.z + patchAButtonOffset.z
        };

        // 3D→UI変換
        DirectX::XMFLOAT2 screenPos = WorldToUI(pos);

        startAButton->SetWorldPosition({ screenPos.x,screenPos.y });
    }
    else
    {
        startAButton->SetVisible(false);
    }


#endif // 0

    if (InputSystem::GetInputState("GamePadA", InputStateMask::Trigger))
    {
        if (!selectableStages.empty())
        {
            auto stage =
                selectableStages[selectedStageIndex];
            // ステージ決定音
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/decide_stage.wav", 1.5f);

            Logger::Log(u8"ステージ選択");

            if (stage->stage == STAGE_NAME::TUTORIAL)
            {
                SceneTransitionManager::SceneTransitionParam param;

                param["preload"] = "TutorialScene";
                param["stage"] = std::string(magic_enum::enum_name(stage->stage));
                param["fromScene"] = "StageSelect";

                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    param
                );


#if 0

                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    {
                        {"preload", "TutorialScene"},
                        {
                            "stage",
                            std::string(
                                magic_enum::enum_name(stage->stage))
                        }
                    });

#endif // 0
            }
            else
            {

                SceneTransitionManager::SceneTransitionParam param;

                param["preload"] = "GameScene";
                param["stage"] = std::string(magic_enum::enum_name(stage->stage));
                param["fromScene"] = "StageSelect";

                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    param
                );



                //SceneTransitionManager::Instance().RequestTransition(
                //    "LoadingScene",
                //    {
                //        {"preload", "GameScene"},
                //        {
                //            "stage",
                //            std::string(
                //                magic_enum::enum_name(stage->stage))
                //        },
                //        {
                //            "fromScene",
                //            "SelectScene"
                //        }
                //    });
            }

        }
    }

}

// コントローラー対応時に選択切り替え処理
void BookBaseActor::MoveSelection(int dir)
{
    if (selectableStages.empty())
        return;

    selectedStageIndex += dir;

    // コントローラー選択切り替え音
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/stage_select_control.wav");

    const int count = static_cast<int>(selectableStages.size());

    if (selectedStageIndex >= count)
    {
        //selectedStageIndex = 0;
        selectedStageIndex = count - 1;
    }
    if (selectedStageIndex < 0)
    {
        selectedStageIndex = count - 1;
    }
}

// ロック中のワッペンの彩度を下げる
void BookBaseActor::UpdateStageVisual(const std::shared_ptr<StageSelectData>& stage)
{
    if (!stage->isUnlocked)
    {
        stage->model->plusAlphaCBuffer->data.saturation = -0.87f;
        stage->model->SetRelativeScaleDirect({ 1,1,1 });
    }
    else
    {
        stage->model->plusAlphaCBuffer->data.saturation = 0.0f;
    }
}