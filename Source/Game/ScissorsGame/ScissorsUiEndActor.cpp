#include "pch.h"
#include "ScissorsUiEndActor.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"

void ScissorsUiEndActor::Initialize(const Transform& transform)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    // クリアのスプライト描画コンポーネントを追加
    finishImageComponent = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/finish.png", "finish_ui");
    finishImageComponent->SetWorldPosition({ 1920.0f * 0.5f, 1080.0f * 0.5f });
    finishImageComponent->SetVisible(true);
    finishImageComponent->SetScale({ 1.0f, 1.0f });
    finishImageComponent->SetSize({ 952, 342 });
    finishImageComponent->SetPivot({ 0.5f,0.5f });
    finishImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
    uiManager->Add(finishImageComponent);

    easingAlpha = std::make_shared<EasingRunner>();
    easingScale = std::make_shared<EasingRunner>();

}

void ScissorsUiEndActor::Update(float deltaTime)
{
    easingAlpha->Tick(deltaTime);
    easingScale->Tick(deltaTime);

    finishImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,easingAlphaValue });
    //float scale = std::lerp(5.0f, 1.0f, easingValue);
    finishImageComponent->SetScale({ easingScaleValue, easingScaleValue });
}

void ScissorsUiEndActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button("PlayFinish"))
    {
        Play();
    }
#endif
};


void ScissorsUiEndActor::Play()
{
    finishImageComponent->SetVisible(true);

#if 0
    // ゲームの入力処理を止める
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            gameManager->SetGameInputEnabled(false);
        }
    }
#endif // 0

    {
        TestEasingHandler handler;
        handler.AddEasing(
            TestEaseType::OutExp,
            0.0f,
            1.0f,
            fadeInTime
        );

        handler.AddWait(0.65f);

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            0.0f,
            fadeOutTime
        );

        handler.SetCompletedFunction([this]()
            {
                finishImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingAlphaValue; };
        accessor.setter = [this](float t)
            {
                easingAlphaValue = t;
            };

        easingAlpha->StartHandler(handler, accessor);
    }


    {
        TestEasingHandler handler;
        handler.AddEasing(
            TestEaseType::OutExp,
            5.0f,
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

        handler.AddWait(0.65f);

        handler.AddEasing(
            TestEaseType::OutExp,
            1.0f,
            1.2f,
            fadeOutTime
        );

        handler.SetCompletedFunction([this]()
            {
                // 終わったらシーン遷移
#if 1
                const char* types[] = { "0", "1" };
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "ResultScene"), std::make_pair("type", types[rand() % 2])  ,std::make_pair("fromScene", "GameScene") });
#else
                const char* types[] = { "0", "1" };
                Scene::_transition("LoadingScene", { std::make_pair("preload", "ResultScene"),{"difficulty", "0"} });

#endif // 0
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingScaleValue; };
        accessor.setter = [this](float t)
            {
                easingScaleValue = t;
            };

        easingScale->StartHandler(handler, accessor);
    }

    // 音を再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/se_end.wav");

}



