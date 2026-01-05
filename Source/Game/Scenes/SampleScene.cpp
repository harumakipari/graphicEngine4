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

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Dessert/Pudding.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"
#include "Game/Actors/Enemy/Boss/BossEnemy.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/Cloth.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/TitleUIFactory.h"

#include "Physics/Physics.h"
#include "Game/Actors/Stage/FightStage.h"

#include "Graphics/PostProcess/BloomEffect.h"
#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"


bool SampleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();
    EventSystem::Initialize();//追加 UI

    clothSimulate = std::make_unique<ClothSimulate>(device, "./Data/Models/ClothFlag/cloth.gltf");
    //clothSimulate = std::make_unique<ClothSimulate>(device, "./Data/Models/TestCloth/cloth1.gltf");

    RegisterRenderHook(RenderPass::Opaque, [&](ID3D11DeviceContext* immediateContext)
        {
            if (const auto cloth = GetActorManager()->GetActorByName("cloth"))
            {
                clothSimulate->Render(immediateContext, cloth->GetWorldTransform());
            }
        });

    return true;
}

void SampleScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM/title.wav");
    audioComp->SetLoop(true);
    audioComp->Play();
    audioComp->SetVolume(0.2f);



    //std::shared_ptr<Sprite> uiSprite = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/UI/icon_chara.png");

    std::shared_ptr<UIImageComponent> image = std::make_shared<UIImageComponent>("./Data/Textures/UI/icon_chara.png", "image");
    image->SetWorldPosition({ 50, 50 });
    image->SetSize({ 200, 200 });

    uiManager->Add(image);


    std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/icon_chara.png", "button");
    button->SetWorldPosition({ 300, 50 });
    button->SetSize({ 200, 80 });

    uiManager->Add(button);

    std::shared_ptr<UIGaugeComponent> gauge = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/icon_chara.png", "gauge");
    gauge->SetWorldPosition({ 50, 300 });
    gauge->SetSize({ 300, 40 });
    gauge->value = 1.0f;

    uiManager->Add(gauge);

    // ボタンでゲージ減らす
    button->onClick = [gauge]()
        {
            Logger::Log(u8"ボタンButton Clicked!");
            Logger::Error(u8"ボタンButton Clicked!");
            Logger::Warning(u8"ボタンButton Clicked!");

            OutputDebugStringA("Button Clicked!\n");
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
            gauge->value -= 0.1f;
            if (gauge->value < 0.0f)
                gauge->value = 0.0f;
        };

}

void SampleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);

    Physics::Instance().Update(Time::UnscaledDeltaTime());
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加
    clothSimulate->Update(deltaTime);
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

//#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "PuddingGameScene"), std::make_pair("type", types[rand() % 2]) });
    }
//#endif // !_DEBUG
}

void SampleScene::SetUpActors()
{
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.3f,1.3f,1.3f });
    auto player = this->GetActorManager()->CreateAndRegisterActorWithTransform<Player>("player", playerTr);
    mainCameraComponent->target = (player->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(.0f);
    //mainCameraComponent->followTarget = (titlePlayer->GetRootComponent());
    //mainCameraComponent->lookAtTarget = (titlePlayer->GetRootComponent());

    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.8f,0.8f,0.8f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<FightStage>("stage", stageTr); // 元のモデルの scale を 0.4f

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    Transform buildTr(DirectX::XMFLOAT3{ -5.0f,-2.45f,3.0 }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto building = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("cloth", buildTr);
    building->AddComponent<StaticMeshComponent>("cloth")->SetModel("./Data/Models/ClothFlag/pole.gltf");
    //building->AddComponent<SkeletalMeshComponent>("pudding")->SetModel("./Data/Models/cherry_pudding/scene.gltf");

    Transform buildTr2(DirectX::XMFLOAT3{ -3.0f,-2.45f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.8f,0.8f,0.8f });
    auto building2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pudding>("building", buildTr2);

    auto pauseActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pause>("pauseActor", buildTr2);

#if 1
    CameraManager::SetGameCamera(mainCameraActor.get());
#else
    CameraManager::SetGameCamera(debugCameraActor.get());
#endif // 0
    //stageCollisionMesh = std::make_shared<CollisionMesh>(Graphics::GetDevice(), "./Data/Models/Stage/stage.gltf", true);

    Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,-2.45f,5.6f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 2.0f,2.0f,2.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<BossEnemy>("enemy", enemyTr);


    CameraManager::SetDebugCamera(debugCameraActor);
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
