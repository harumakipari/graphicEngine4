#include "pch.h"
#include "TitleBookActor.h"

void TitleBookActor::Initialize(const Transform& transform)
{
    std::string parentName = "TitleBookActor";
    auto rootComponent= AddComponent<SceneComponent>(parentName);

    bookRightModel = AddComponent<SkeletalMeshComponent>("bookRightModel", parentName);
    bookRightModel->SetModel("./Data/TeamModels/Title/BookRight.gltf", false, false);


    bookLeftModel = AddComponent<SkeletalMeshComponent>("bookLeftModel",parentName);
    bookLeftModel->SetModel("./Data/TeamModels/Title/BookLeft.gltf", false, false);
    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });

    easingRunner = std::make_unique<EasingRunner>();

    patchTutorialModel = AddComponent<SkeletalMeshComponent>("patchTutorialModel", "bookLeftModel");
    patchTutorialModel->SetModel("./Data/TeamModels/Title/patchModel_1.gltf", false, false);
    patchTutorialModel->SetRelativeLocationDirect({ 3.3f,0.0f,-1.5f });

    
}

void TitleBookActor::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);

    bookLeftModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,angle });
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
    startEuler = 180.0f;
    endEuler = 0.0f;

    

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