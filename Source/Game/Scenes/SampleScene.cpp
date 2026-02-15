#include "pch.h"
#include "SampleScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Dessert/Pudding.h"
#include "Game/Actors/Enemy/Boss/BossEnemy.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/Cloth.h"


#include "Physics/Physics.h"
#include "Game/DarkGame/DarkActors/FightStage.h"


#include "Graphics/PostProcess/BloomEffect.h"
#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"


bool SampleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    // ライトの方向と色を設定
    lightDirection = { 0.018f, -0.095f, -0.094f, 0.0f };
    lightColor = { 1.0f, 1.0f, 1.0f, 5.62f };

    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();

    return true;
}

void SampleScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM/game.wav");
    audioComp->SetLoop(true);
    audioComp->Play();
    //audioComp->SetVolume(0.2f);

    std::shared_ptr<UIImageComponent> image = std::make_shared<UIImageComponent>("./Data/Textures/UI/icon_chara.png", "image");
    image->SetWorldPosition({ 50, 50 });
    image->SetSize({ 200, 200 });

    uiManager->Add(image);


    std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/icon_chara.png", "button");
    button->SetWorldPosition({ 300, 50 });
    button->SetSize({ 200, 80 });

    uiManager->Add(button);

    std::shared_ptr<UIGaugeComponent> gauge = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/boss_hp_frame.png", "./Data/Textures/UI/boss_hp.png", "gauge");
    gauge->SetWorldPosition({ 50, 300 });
    gauge->SetSize({ 300, 40 });

    uiManager->Add(gauge);

    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();

}

void SampleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);

    Physics::Instance().Update(Time::UnscaledDeltaTime());
    clothSimulate->Update(deltaTime);
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

    float followSpeed = 6.0f; // 調整用

#if 0
    if (player && mainCameraComponent)
    {
        const auto& forward = player->GetForward();
        float playerYaw = std::atan2f(forward.x, forward.z);
        //mainCameraComponent->yaw = (playerYaw);

        float delta = playerYaw - mainCameraComponent->yaw;
        delta = std::atan2f(std::sinf(delta), std::cosf(delta)); // -3.14 ~ 3.14
        mainCameraComponent->yaw += delta * followSpeed * deltaTime;

    }
#endif // 0

    //#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("PuddingGameScene");

        //Scene::_transition("LoadingScene", { std::make_pair("preload", "PuddingGameScene"), std::make_pair("type", types[rand() % 2]) });
    }
    //#endif // !_DEBUG
}

void SampleScene::SetUpActors()
{
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    Transform playerTr(DirectX::XMFLOAT3{ 12.0f,0.0f,10.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,10.0f }, DirectX::XMFLOAT3{ 1.07f,1.07f,1.07f });
    player = this->GetActorManager()->CreateAndRegisterActorWithTransform<Player>("player", playerTr);
    mainCameraComponent->target = (player->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(.0f);
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("sampleシーンのカメラ設定される。"));

    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<FightStage>("stage", stageTr); // 元のモデルの scale を 0.4f

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    auto pauseActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pause>("pauseActor");

    //Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,-2.45f,5.6f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 2.0f,2.0f,2.0f });
    //auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<BossEnemy>("enemy", enemyTr);

    cameraManager->SetDebugCamera(debugCameraActor);
}

bool SampleScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void SampleScene::DrawGui()
{
    SceneBase::DrawGui();
}
