#include "pch.h"
#include "ScissorsUIStartActor.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"


void ScissorsUIStartActor::Initialize(const Transform& transform)
{
    targetPos = { 1920.0f * 0.5f, 1080.0f * 0.5f };


    easingAlpha = std::make_shared<EasingRunner>();
    easingPosition = std::make_shared<EasingRunner>();
    easingGoAlpha = std::make_shared<EasingRunner>();
    easingGoScale = std::make_shared<EasingRunner>();
}


// テクスチャをステージに応じて切り替える
void ScissorsUIStartActor::SetTexture(STAGE_NAME stageName)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    std::unordered_map<STAGE_NAME, std::string> stageTimeTable =
    {
        { STAGE_NAME::TUTORIAL,    "./Data/Textures/ScissorsUI/stage_first_start.png" },   // チュートリアルはボーナス無しでもOK
        { STAGE_NAME::FIRST,         "./Data/Textures/ScissorsUI/stage_first_start.png" },
        { STAGE_NAME::BOBBIN_FIRST,  "./Data/Textures/ScissorsUI/stage_bobbin_start.png" },
        { STAGE_NAME::REFLECT_WALL,  "./Data/Textures/ScissorsUI/stage_redirect_start.png" },
        { STAGE_NAME::BOBBIN_SECOND, "./Data/Textures/ScissorsUI/stage_first_start.png"} ,
        { STAGE_NAME::DIFFICULT,     "./Data/Textures/ScissorsUI/stage_difficult_start.png" },
        { STAGE_NAME::BOSS,          "./Data/Textures/ScissorsUI/stage_boss_start.png" },
    };


    std::string textureName = stageTimeTable[stageName];

    readyImageComponent = std::make_shared<UIImageComponent>(textureName, "ready_ui");
    readyImageComponent->SetWorldPosition({ 0.0f, targetPos.y });
    readyImageComponent->SetVisible(true);
    readyImageComponent->SetScale({ 1.0f, 1.0f });
    readyImageComponent->SetSize({ 952, 342 });
    readyImageComponent->SetPivot({ 0.5f,0.5f });
    readyImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
    uiManager->Add(readyImageComponent);

    goImageComponent = std::make_shared<UIImageComponent>(textureName, "go_ui");
    goImageComponent->SetWorldPosition({ 1920.0f * 0.5f, 1080.0f * 0.5f });
    goImageComponent->SetVisible(true);
    goImageComponent->SetScale({ 1.0f, 1.0f });
    goImageComponent->SetSize({ 952, 342 });
    goImageComponent->SetPivot({ 0.5f,0.5f });
    goImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
    uiManager->Add(goImageComponent);

}

void ScissorsUIStartActor::Update(float deltaTime)
{
    easingAlpha->Tick(deltaTime);
    easingPosition->Tick(deltaTime);
    easingGoAlpha->Tick(deltaTime);
    easingGoScale->Tick(deltaTime);

    readyImageComponent->SetColor(XMFLOAT4{ 1,1,1,easingReadyAlphaValue });
    readyImageComponent->SetWorldPosition({ easingReadyPositionValue,targetPos.y });

    goImageComponent->SetColor(XMFLOAT4{ 1,1,1,easingGoAlphaValue });
    goImageComponent->SetScale({ easingGoScaleValue,easingGoScaleValue });
}

void ScissorsUIStartActor::DrawImGuiDetails()
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
    ImGui::DragFloat(U8("readyとgoの間の時間"), &waitInterval, 0.05f, 0.1f, 1.0f);
#endif
};


void ScissorsUIStartActor::PlayReady(const std::function<void()>& onFinished)
{
    // 
    onStartFinished = onFinished;

    // 透明度の easing
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
            1.0f,
            fadeOutTime
        );

        handler.AddWait(waitInterval);

        handler.SetCompletedFunction([this]()
            {
                readyImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f });
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

    // position の easing
    {
        TestEasingHandler handler;

        handler.AddEasing(
            TestEaseType::OutExp,
            0.0f,
            targetPos.x,
            fadeInTime
        );

        handler.AddWait(waitInterval);


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


    // 音を再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/se_ready.wav");    // ready 

    // ゲームの入力処理を止める
#if 0
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            gameManager->SetGameInputEnabled(false);
        }
    }
#endif // 0
}



void ScissorsUIStartActor::PlayGo()
{
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/se_start.wav");    // go

    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.1f);

        handler.AddEasing(
            TestEaseType::OutExp,
            targetPos.x,
            2400.0f,
            fadeInTime
        );

        handler.SetCompletedFunction([this]()
            {
                readyImageComponent->SetWorldPosition({ 2400.0f,targetPos.y });
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingReadyPositionValue; };
        accessor.setter = [this](float t)
            {
                easingReadyPositionValue = t;
            };

        easingGoAlpha->StartHandler(handler, accessor);
    }

    // 透明度の easing
    {
        TestEasingHandler handler;
        handler.AddWait(0.1f);

        handler.AddEasing(
            TestEaseType::InSine,
            1.0f,
            0.0f,
            fadeInTime
        );

        handler.SetCompletedFunction([this]()
            {
                readyImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
                // ★ 演出完了通知
                if (onStartFinished)
                {
                    onStartFinished();
                }

                //// ゲームの入力処理を解禁する
                //if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
                //{
                //    if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                //    {
                //        gameManager->SetGameInputEnabled(true);
                //    }
                //}
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingReadyAlphaValue; };
        accessor.setter = [this](float t)
            {
                easingReadyAlphaValue = t;
            };

        easingGoScale->StartHandler(handler, accessor);
    }



#if 0
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
                goImageComponent->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f });
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
                // ★ 演出完了通知
                if (onStartFinished)
                {
                    onStartFinished();
                }

                // ゲームの入力処理を解禁する
                if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
                {
                    if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                    {
                        gameManager->SetGameInputEnabled(true);
                    }
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


#endif // 0



}
