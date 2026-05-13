#include "pch.h"
#include "BookBaseActor.h"

#include <magic_enum.hpp>

#include "Engine/Audio/CoreAudio.h"
#include "Physics/CollisionFunction.h"
#include "UI/Game/SceneTransitionManager.h"

void BookBaseActor::Initialize(const Transform& transform)
{
    std::string parentName = "TitleBookActor";
    auto rootComponent = AddComponent<SceneComponent>(parentName);

    bookRightModel = AddComponent<SkeletalMeshComponent>("bookRightModel", parentName);
    bookRightModel->SetModel("./Data/TeamModels/Title/BookRight.gltf", false, false);

    bookLeftModel = AddComponent<SkeletalMeshComponent>("bookLeftModel", parentName);
    bookLeftModel->SetModel("./Data/TeamModels/Title/BookLeft.gltf", false, false);
    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });

    // 真ん中モデルを追加
    bookMiddleModel = AddComponent<SkeletalMeshComponent>("bookMiddleModel", parentName);
    bookMiddleModel->SetModel("./Data/TeamModels/Title/BookMiddle.gltf", false, false);
    bookMiddleModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookMiddleModel->SetIsCastShadow(false);

    // 背表紙モデル
    bookSpineModel = AddComponent<SkeletalMeshComponent>("bookSpineModel", parentName);
    bookSpineModel->SetModel("./Data/TeamModels/Title/BookSpineModel.gltf", false, false);
    bookSpineModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookSpineModel->SetRelativeLocationDirect({ 0.1f,0.0f,0.0f });
    bookSpineModel->SetIsCastShadow(false);

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
        STAGE_NAME::DIFFICULT,
        "./Data/TeamModels/Title/patchModelDifficult.gltf",
        { -1.3f,0.1f,-1.6f });


    CreateStagePatch(
        rightPage,
        STAGE_NAME::BOSS,
        "./Data/TeamModels/Title/patchModelBoss.gltf",
        { -3.4f,0.1f,1.7f });

#endif // 0
}

void BookBaseActor::Update(float deltaTime)
{
    easingOneRunner->Tick(deltaTime);
    easingTwoRunner->Tick(deltaTime);

    if (bookState == BookPageState::FirstPage)
    {
        UpdatePage(leftPage);
        UpdatePage(rightPage);
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
        Logger::Log("bookTwoAlpha"+std::to_string(bookTwoAlpha));
        for (auto& stage : leftPage.stages)
        {// 左ページのワッペン
            // ワッペンの位置を下げる
            float patchOffsetY = std::lerp(openPatchPosY, closePatchPosY, bookTwoAlpha);
            stage.model->SetRelativeLocationDirect({ stage.offsetPos.x,patchOffsetY, stage.offsetPos.z });
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
#endif
}

// 本を開く
void BookBaseActor::OpenBook(float interval)
{
    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            bookOneAlpha = 1.0f;
            bookState = BookPageState::FirstPage;
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
void BookBaseActor::OpenSecondPage(float interval)
{
    easingTwoRunner->Clear();

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            bookTwoAlpha = 1.0f;
            bookState = BookPageState::SecondPage;
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

    auto box =
        AddComponent<BoxComponent>(
            boxName,
            name);

    DirectX::XMFLOAT3 size = model->GetModelSize();
    box->SetBoxExtent(size);
    box->SetLayer(CollisionLayer::WorldStatic);
    box->Initialize();

    page.stages.push_back(
        {
            stage,
            model,
            box,
            pos
        });
}

// ページのパッチの更新処理
void BookBaseActor::UpdatePage(BookPage& page)
{
    DirectX::XMFLOAT2 cursor;

    // マウスが画面外
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

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
            result.component == stage.collider.get();

        // ホバー演出
        if (hitThis)
        {
            stage.model->SetRelativeScaleDirect(
                {
                    1.1f,
                    1.1f,
                    1.1f
                });
        }
        else
        {
            stage.model->SetRelativeScaleDirect(
                {
                    1.0f,
                    1.0f,
                    1.0f
                });
        }

        // クリック
        if (hitThis &&
            InputSystem::GetInputState(
                "MouseLeft",
                InputStateMask::Trigger))
        {
            Logger::Log(u8"ステージ選択");

            if (stage.stage == STAGE_NAME::TUTORIAL)
            {// チュートリアルを選択した時のみ
                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    {
                        {"preload", "TutorialScene"},
                        {
                            "stage",
                            std::string(
                                magic_enum::enum_name(stage.stage))
                        }
                    });
            }
            else
            {
                SceneTransitionManager::Instance().RequestTransition(
                    "LoadingScene",
                    {
                        {"preload", "GameScene"},
                        {
                            "stage",
                            std::string(
                                magic_enum::enum_name(stage.stage))
                        }
                    });

            }
            CoreAudio::PlayOneShot(
                L"./Data/Sound/SE/push_button.wav");
        }
    }
}