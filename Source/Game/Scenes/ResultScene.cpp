#include "pch.h"
#include "ResultScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include <magic_enum.hpp>

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"
#include "Game/OdenGame/OdenResultScoreActor.h"
#include "Game/OdenGame/OdenActors/OdenResultStageActor.h"


#include "Physics/Physics.h"

#include "Physics/CollisionSystem.h"



bool ResultScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();

    return true;
}

void ResultScene::Start()
{
    // BGM再生
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM/result.wav");
    audioComp->SetLoop(true);
    audioComp->Play();

#if 1
    // タイトルへ戻るボタンを生成
    {
        std::shared_ptr<UIButtonComponent> backToTitleButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/Result/back_to_title.png", "backToTitleButton");
        backToTitleButton->SetWorldPosition({ 300, 50 });
        backToTitleButton->SetSize({ 200, 80 });

        uiManager->Add(backToTitleButton);

        // タイトルへ戻るボタンがクリックされたときの処理
        backToTitleButton->onClick = []()
            {
                Logger::Log(u8"BackToTitleボタンButton Clicked!");
                static float  value = 1.0f;
                const char* types[] = { "0", "1" };
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("type", types[rand() % 2]) });
                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }


    // リトライボタンを生成
    {
        std::shared_ptr<UIButtonComponent> retryButton = std::make_shared<UIButtonComponent>("./Data/Textures/UI/Result/retry.png", "retryButton");
        retryButton->SetWorldPosition({ 300, 250 });
        retryButton->SetSize({ 200, 80 });

        uiManager->Add(retryButton);

        // タイトルへ戻るボタンがクリックされたときの処理
        retryButton->onClick = []()
            {
                Logger::Log(u8"retryButton Clicked!");
                const char* types[] = { "0", "1" };
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    backImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/backGround.png", "backGround");
    backImage->SetSize({ 1920, 1080 });

    menuImage = std::make_shared<UIImageComponent>("menuImage");
    menuImage->SetWorldPosition({ 700, 260 });
    menuImage->SetSize({ 650, 750 });
    //uiManager->Add(menuImage);


    // ここで背景を描画
    RegisterRenderHook(RenderPass::Sky, [&](ID3D11DeviceContext* immediateContext)
        {
            backImage->Draw(immediateContext);
            menuImage->Draw(immediateContext);
        });

    // 結果スコア表示アクターを生成
    auto resultActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultScoreActor>("resultActor");
    resultActor->SetFontAndMakeTextComponent();

    Transform stageTr(DirectX::XMFLOAT3{ 0.f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stageActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultStageActor>("resultStage", stageTr);


#endif // 1

    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();
}

void ResultScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);
    Physics::Instance().Update(Time::UnscaledDeltaTime());
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

}

void ResultScene::SetUpActors()
{
    // メインカメラのターゲットアクターを生成
    Transform cameraTargetTr(DirectX::XMFLOAT3{ 0.6f,11.4f,-15.4f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto mainCameraTarget = GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("MainCameraActorTarget", cameraTargetTr);

    // メインカメラアクターを生成
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->target = (mainCameraTarget->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(0.0f);
    mainCameraComponent->yaw = DirectX::XMConvertToRadians(1.5f);
    mainCameraComponent->distance = 30.6f;
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("MainSceneのカメラ設定される。"));

#ifdef _DEBUG
    // デバックカメラアクターを生成
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });
    cameraManager->SetDebugCamera(debugCameraActor);
#endif // !_DEBUG

    // ステージアクターを生成
#if 0
    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.5f,0.5f,0.5f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenResultStageActor>("stage", stageTr);

#endif // 0

}

void ResultScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    SceneBase::Render(immediateContext, deltaTime);
}


bool ResultScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void ResultScene::DrawGui()
{
#ifdef USE_IMGUI
    SceneBase::DrawGui();

#endif
}
