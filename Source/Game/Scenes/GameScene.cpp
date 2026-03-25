#include "pch.h"
#include "GameScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#endif

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"

#include "Game/Actors/Dessert/Pudding.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/Cloth.h"


#include "Physics/Physics.h"
#include "Game/DarkGame/DarkActors/DarkStage.h"
#include "Game/DarkGame/DarkActors/DarkStageChandelierActor.h"
#include "Game/DarkGame/DarkActors/DoorActor.h"
#include "Game/DarkGame/DarkActors/DarkEnemy/SkeletonWarriorEnemy.h"


#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"


bool GameScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    lightDirection = { 0.722f, -0.38f, -0.0211f, 0.9f };   // 上の窓からの光
    lightColor = { 1.0f, 0.8f, 1.0f, 2.6f };
    {
        SceneBase::Initialize(device, width, height, props);
    }
    {
        Physics::Instance().Initialize();
    }
    {
        SetUpActors();
    }

    return true;
}

void GameScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM/game.wav");
    audioComp->SetLoop(true);
    audioComp->Play();
    //audioComp->SetVolume(0.2f);
    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();

}

void GameScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);

    Physics::Instance().Update(Time::UnscaledDeltaTime());
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();


    //#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        SceneTransitionManager::Instance().RequestTransition("SampleScene");

        //Scene::_transition("LoadingScene", { std::make_pair("preload", "PuddingGameScene"), std::make_pair("type", types[rand() % 2]) });
    }
    //#endif // !_DEBUG
}

// 定数バッファの更新処理をシーンごとにカスタマイズできるようにするための仮想関数
void GameScene::UpdateConstants(ID3D11DeviceContext* immediateContext, float deltaTime)
{

}

void GameScene::SetUpActors()
{
    Transform mainCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor", mainCameraTr);
    mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    {
        PROFILE_SCOPE("Create Player");
        Transform playerTr(DirectX::XMFLOAT3{ 40.0f,0.0f,-24.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,10.0f }, DirectX::XMFLOAT3{ 1.07f,1.07f,1.07f });
        auto player = this->GetActorManager()->CreateAndRegisterActorWithTransform<Player>("player", playerTr);
        mainCameraActor->SetTarget(player->GetRootComponent());
    }
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("sampleシーンのカメラ設定される。"));

    Transform debugCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam", debugCameraTr);
    cameraManager->SetDebugCamera(debugCameraActor);

    Transform cinemaCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto cinemaCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<CinemaCamera>("cinemaCam", cinemaCameraTr);
    cameraManager->SetCinematicCamera(cinemaCameraActor);

    Transform movieCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto movieCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MovieCamera>("movieCam", movieCameraTr);
    cameraManager->SetMovieCamera(movieCameraActor);

    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("stage", movieCameraTr);
    auto staticMeshCom = stage->AddComponent<StaticMeshComponent>("stage");
    staticMeshCom->SetModel("./Data/Models/SoulStage0325/DarkSoulStage.gltf");

}

bool GameScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void GameScene::DrawGui()
{
#ifdef USE_IMGUI
    SceneBase::DrawGui();
#endif

}
