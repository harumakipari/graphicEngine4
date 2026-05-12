#include "pch.h"
#include "TitleBookActor.h"

#include <magic_enum.hpp>

#include "Engine/Audio/CoreAudio.h"
#include "Physics/CollisionFunction.h"
#include "UI/Game/SceneTransitionManager.h"

void TitleBookActor::Initialize(const Transform& transform)
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

    // 背表紙モデル
    bookSpineModel = AddComponent<SkeletalMeshComponent>("bookSpineModel", parentName);
    bookSpineModel->SetModel("./Data/TeamModels/Title/BookSpineModel.gltf", false, false);
    bookSpineModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    bookSpineModel->SetRelativeLocationDirect({ 0.1f,0.0f,0.0f });

    easingRunner = std::make_unique<EasingRunner>();


#if 1
    // 左の本モデル
    leftPage.parentName = "bookLeftModel";
    // 右の本モデル
    rightPage.parentName = "bookRightModel";

    CreateStagePatch(
        leftPage,
        STAGE_NAME::FIRST,
        "./Data/TeamModels/Title/patchModelFirst.gltf",
        { 3.3f,0.0f,-1.5f });

    CreateStagePatch(
        leftPage,
        STAGE_NAME::BOBBIN_FIRST,
        "./Data/TeamModels/Title/patchModelBobbinFirst.gltf",
        { 1.0f,0.0f,-1.5f });

    //CreateStagePatch(
    //    rightPage,
    //    STAGE_NAME::BOSS,
    //    "./Data/TeamModels/Title/patchModel_7.gltf",
    //    { -2.0f,0.0f,-1.5f });

#endif // 0
    //patchTutorialModel = AddComponent<SkeletalMeshComponent>("patchTutorialModel", "bookLeftModel");
    //patchTutorialModel->SetModel("./Data/TeamModels/Title/patchModelFirst.gltf", false, false);
    //patchTutorialModel->SetRelativeLocationDirect({ 3.3f,0.0f,-1.5f });
}

void TitleBookActor::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);
    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,angle });
    //bookSpineModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,angle  });

    //UpdatePage(leftPage);
    //UpdatePage(rightPage);


}

void TitleBookActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("本が閉じる")))
    {
        startEuler = 0.0f;
        endEuler = 180.0f;
        Play(2.0f);
    }
    if (ImGui::Button(U8("本が開く")))
    {
        startEuler = 180.0f;
        endEuler = 0.0f;
        Play(2.0f);
    }
#endif
}

// 本を開く
void TitleBookActor::Play(float interval)
{
    // 本を開く
    startEuler = 180.0f;
    endEuler = 0.0f;

    // book の角度 の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            startEuler,
            endEuler,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                angle = endEuler;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return angle; };
        accessor.setter = [this](float t)
            {
                angle = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }


}

// 本を閉じる
void TitleBookActor::PlayReverse(float interval)
{
    startEuler = 0.0f;
    endEuler = 180.0f;

    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            startEuler,
            endEuler,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                angle = endEuler;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return angle; };
        accessor.setter = [this](float t)
            {
                angle = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }
}

// ステージパッチを生成する
void TitleBookActor::CreateStagePatch(TitleBookActor::BookPage& page, STAGE_NAME stage, const char* modelPath, const DirectX::XMFLOAT3& pos)
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
            box
        });

}

// ページのパッチの更新処理
void TitleBookActor::UpdatePage(BookPage& page)
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

            CoreAudio::PlayOneShot(
                L"./Data/Sound/SE/push_button.wav");
        }
    }
}