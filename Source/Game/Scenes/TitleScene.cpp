#include "pch.h"
#include "TitleScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include <magic_enum.hpp>

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"



#include "Physics/Physics.h"

#include "Physics/CollisionSystem.h"



bool TitleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();

    return true;
}

void TitleScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM/title.wav");
    audioComp->SetLoop(true);
    audioComp->Play();

    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();
}

void TitleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);
    Physics::Instance().Update(Time::UnscaledDeltaTime());
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();
}

void TitleScene::SetUpActors()
{
    // メインカメラのターゲットアクターを生成
    Transform cameraTargetTr(DirectX::XMFLOAT3{ 5.4f,0.0f,4.3f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto mainCameraTarget = GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("MainCameraActorTarget", cameraTargetTr);

    // メインカメラアクターを生成
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->target = (mainCameraTarget->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(51.5f);
    mainCameraComponent->distance = 18.4f;
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("MainSceneのカメラ設定される。"));

#ifdef _DEBUG
    // デバックカメラアクターを生成
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });
    cameraManager->SetDebugCamera(debugCameraActor);
#endif // !_DEBUG
}

void TitleScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    SceneBase::Render(immediateContext, deltaTime);
}


bool TitleScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void TitleScene::DrawGui()
{
#ifdef USE_IMGUI
    SceneBase::DrawGui();

#endif
}
