#include "pch.h"
#include "OdenUIStartActor.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"


void OdenUIStartActor::Initialize(const Transform& transform)
{
    auto uiManager = GetOwnerScene()->GetUIManager();
    targetPos = { 1920.0f * 0.5f, 1080.0f * 0.5f };
    // 
    readyImageComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/ready.png", "ready_ui");
    readyImageComponent->SetWorldPosition({ 0.0f, targetPos.y });
    readyImageComponent->SetVisible(true);
    readyImageComponent->SetScale({ 1.0f, 1.0f });
    readyImageComponent->SetSize({ 895.0f, 512.0f });
    readyImageComponent->SetPivot({ 0.5f,0.5f });
    readyImageComponent->SetColor({ 1.0f,1.0f,1.0f,0.0f });
    uiManager->Add(readyImageComponent);

    goImageComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/go.png", "go_ui");
    goImageComponent->SetWorldPosition({ 1920.0f * 0.5f, 1080.0f * 0.5f });
    goImageComponent->SetVisible(true);
    goImageComponent->SetScale({ 1.0f, 1.0f });
    goImageComponent->SetSize({ 895.0f, 512.0f });
    goImageComponent->SetPivot({ 0.5f,0.5f });
    goImageComponent->SetColor({ 1.0f,1.0f,1.0f,0.0f });
    uiManager->Add(goImageComponent);

    easingAlpha = std::make_shared<EasingRunner>();
    easingPosition = std::make_shared<EasingRunner>();
    easingGoAlpha = std::make_shared<EasingRunner>();
    easingGoScale = std::make_shared<EasingRunner>();

}

void OdenUIStartActor::Update(float deltaTime)
{
    easingAlpha->Tick(deltaTime);
    easingPosition->Tick(deltaTime);
    easingGoAlpha->Tick(deltaTime);
    easingGoScale->Tick(deltaTime);

    readyImageComponent->SetColor({ 1,1,1,easingReadyAlphaValue });
    readyImageComponent->SetWorldPosition({ easingReadyPositionValue,targetPos.y });

    goImageComponent->SetColor({ 1,1,1,easingGoAlphaValue });
    goImageComponent->SetScale({ easingGoScaleValue,easingGoScaleValue });
}

void OdenUIStartActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button("PlayReady"))
    {
        PlayReady();
    }
    if (ImGui::Button("PlayGo"))
    {
        PlayGo();
    }
#endif
};


void OdenUIStartActor::PlayReady(const std::function<void()>& onFinished)
{
    // 
    onStartFinished = onFinished;

    // ìßñæìxÇÃ easing
    {
        TestEasingHandler handler;

        handler.AddEasing(
            TestEaseType::InSine,
            0.0f,
            1.0f,
            fadeInTime
        );

        handler.AddWait(0.3f);

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            0.0f,
            fadeOutTime
        );

        handler.SetCompletedFunction([this]()
            {
                readyImageComponent->SetColor({ 1.0f,1.0f,1.0f,0.0f });
                PlayGo();
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingReadyAlphaValue; };
        accessor.setter = [this](float t)
            {
                easingReadyAlphaValue = t;
            };

        easingAlpha->StartHandler(handler, accessor);
    }

    // position ÇÃ easing
    {
        TestEasingHandler handler;

        handler.AddEasing(
            TestEaseType::OutExp,
            0.0f,
            targetPos.x,
            fadeInTime
        );

        handler.SetCompletedFunction([this]()
            {
                readyImageComponent->SetWorldPosition(targetPos);
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingReadyPositionValue; };
        accessor.setter = [this](float t)
            {
                easingReadyPositionValue = t;
            };

        easingPosition->StartHandler(handler, accessor);
    }


    // âπÇçƒê∂
    CoreAudio::PlayOneShot(L"./Data/Sound/SE/ready_se.wav");    // ready 

}



void OdenUIStartActor::PlayGo()
{
    CoreAudio::PlayOneShot(L"./Data/Sound/SE/se_start.wav");    // go

    {
        TestEasingHandler handler;
        handler.AddEasing(
            TestEaseType::OutExp,
            0.0f,
            1.0f,
            fadeInTime
        );

        handler.AddWait(0.3f);

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            0.0f,
            fadeOutTime
        );

        handler.SetCompletedFunction([this]()
            {
                goImageComponent->SetColor({ 1.0f,1.0f,1.0f,0.0f });
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingGoAlphaValue; };
        accessor.setter = [this](float t)
            {
                easingGoAlphaValue = t;
            };

        easingGoAlpha->StartHandler(handler, accessor);
    }


    {
        TestEasingHandler handler;
        handler.AddEasing(
            TestEaseType::OutExp,
            3.0f,
            0.9f,
            fadeInTime * 0.6f
        );

        handler.AddEasing(
            TestEaseType::OutQuad,
            0.9f,
            1.1f,
            fadeInTime * 0.2f
        );

        handler.AddEasing(
            TestEaseType::OutQuad,
            1.1f,
            1.0f,
            fadeInTime * 0.2f
        );

        handler.AddWait(0.3f);

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            2.0f,
            fadeOutTime
        );

        handler.SetCompletedFunction([this]()
            {
                // Åö ââèoäÆóπí ím
                if (onStartFinished)
                {
                    onStartFinished();
                }
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingGoScaleValue; };
        accessor.setter = [this](float t)
            {
                easingGoScaleValue = t;
            };

        easingGoScale->StartHandler(handler, accessor);
    }




}
