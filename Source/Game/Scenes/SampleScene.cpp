#include "pch.h"
#include "SampleScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"
#include "Game/Actors/Enemy/Boss/BossEnemy.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/Cloth.h"
#include "Game/SofyBody/MassPoint.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/TitleUIFactory.h"

#include "Physics/Physics.h"
#include "Game/Actors/Stage/FightStage.h"

#include "Graphics/PostProcess/BloomEffect.h"
#include "Physics/CollisionSystem.h"


bool SampleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();
    EventSystem::Initialize();//追加 UI
    return true;
}

void SampleScene::Start()
{
}

void SampleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);

    Physics::Instance().Update(deltaTime);
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加

    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();




#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
    }
#endif // !_DEBUG

}

void SampleScene::SetUpActors()
{
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto player = this->GetActorManager()->CreateAndRegisterActorWithTransform<Player>("player", playerTr);
    mainCameraComponent->target = (player->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(19.0f);
    //mainCameraComponent->followTarget = (titlePlayer->GetRootComponent());
    //mainCameraComponent->lookAtTarget = (titlePlayer->GetRootComponent());

    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<FightStage>("stage", stageTr);

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    Transform buildTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto building = this->GetActorManager()->CreateAndRegisterActorWithTransform<ElasticBuilding>("building", buildTr);

#if 1
    CameraManager::SetGameCamera(mainCameraActor.get());
#else
    CameraManager::SetGameCamera(debugCameraActor.get());
#endif // 0
    //stageCollisionMesh = std::make_shared<CollisionMesh>(Graphics::GetDevice(), "./Data/Models/Stage/stage.gltf", true);

    Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<BossEnemy>("enemy", enemyTr);


    CameraManager::SetDebugCamera(debugCameraActor);
}

bool SampleScene::Uninitialize(ID3D11Device* device)
{
    Physics::Instance().Finalize();
    return true;
}

void SampleScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    SceneBase::Render(immediateContext, deltaTime);
}

void SampleScene::DrawGui()
{
    SceneBase::DrawGui();
}
