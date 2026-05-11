#include "pch.h"
#include "TitleCameraTargetActor.h"

void TitleCameraTargetActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_unique<EasingRunner>();
}

void TitleCameraTargetActor::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);
    currentPosition = MathHelper::Lerp(startPosition, endPosition, easingValue);
    SetPosition(currentPosition);
}

void TitleCameraTargetActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("本が閉じる")))
    {
        Play(2.0f);
    }
    if (ImGui::Button(U8("本が開く")))
    {
        PlayReverse(2.0f);
    }
#endif
}

// 
void TitleCameraTargetActor::Play(float interval)
{
    easingValue = 0.0f;
    startPosition = titlePosition;
    endPosition = selectPosition;

    // position の easing
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
                easingValue =1.0f;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingValue; };
        accessor.setter = [this](float t)
            {
                easingValue = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }
}


void TitleCameraTargetActor::PlayReverse(float interval)
{
    easingValue = 0.0f;
    startPosition = selectPosition;
    endPosition = titlePosition;

    // position の easing
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
                easingValue = 1.0f;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingValue; };
        accessor.setter = [this](float t)
            {
                easingValue = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }
}

// タイトル画面に戻る
void TitleCameraTargetActor::SetTitle(bool isTitle)
{
    if (isTitle)
    {
        currentPosition = titlePosition;
        startPosition = currentPosition;
        endPosition = selectPosition;
    }
    else
    {
        currentPosition = selectPosition;
        startPosition = currentPosition;
        endPosition = titlePosition;
    }
}
