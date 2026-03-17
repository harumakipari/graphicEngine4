#include "pch.h"
#include "MorphScene.h"

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
#include "Game/DarkGame/DarkActors/DarkStage.h"

#include "Game/Actors/WaterSphere/WaterSphere.h"
#include "Game/DarkGame/DarkActors/DarkEnemy/SkeletonWarriorEnemy.h"

#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"

bool MorphScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();

#if 0
    morphModel = std::make_unique<MorphModel>(device, "./Data/Models/Morph/morphSphere.gltf");

    RegisterRenderHook(RenderPass::Opaque, [&](ID3D11DeviceContext* immediateContext)
        {
            if (const auto cloth = GetActorManager()->GetActorByName("cloth"))
            {
                morphModel->Render(immediateContext, cloth->GetWorldTransform(), {}, MorphModel::RenderPass::All);
            }
        });
#endif // 0
    shapeMatchingModel = std::make_unique<ShapeMatchingModel>(device, "./Data/Models/Morph/sphere.glb");

    RegisterRenderHook(RenderPass::Opaque, [&](ID3D11DeviceContext* immediateContext)
        {
            if (const auto cloth = GetActorManager()->GetActorByName("pauseActor"))
            {
                //shapeMatchingModel->Render(immediateContext, cloth->GetWorldTransform(), {}, ShapeMatchingModel::RenderPass::Opaque);
            }
        });

    return true;
}

void MorphScene::Start()
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

    std::shared_ptr<UIGaugeComponent> gauge = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/boss_hp_frame.png", "./Data/Textures/UI/boss_hp.png", "gauge");
    gauge->SetWorldPosition({ 50, 300 });
    gauge->SetSize({ 300, 40 });

    uiManager->Add(gauge);

    // ボタンでゲージ減らす
    button->onClick = [gauge]()
        {
            static float value = 1.0f;
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
            value -= 0.1f;
            if (value < 0.0f)
                value = 0.0f;
            gauge->SetValue(value, 1.0f);
        };

    // シーンが切り替わった時に
   // SceneTransitionManager::Instance().NotifySceneChanged();

}

void MorphScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);
    //shapeMatchingModel->Update(deltaTime);

    Physics::Instance().Update(Time::UnscaledDeltaTime());
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

void MorphScene::SetUpActors()
{
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();

    Transform enemyTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,180.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<SkeletonWarriorActor>("enemy", enemyTr);

    mainCameraActor->SetTarget(enemy->GetRootComponent());
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("morphシーンのカメラ設定される。"));

    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<Stage>("stage", stageTr);

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    Transform buildTr(DirectX::XMFLOAT3{ -5.0f,1.0f,3.0 }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto building = this->GetActorManager()->CreateAndRegisterActorWithTransform<WaterSphere>("morphModel", buildTr);
    auto sphere = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("sphere", buildTr);
    auto model = sphere->AddComponent<SkeletalMeshComponent>("skeletalMesh");
    model->SetModel("./Data/Models/Primitives/sphere.glb");



    //building->AddComponent<StaticMeshComponent>("cloth")->SetModel("./Data/Models/ClothFlag/pole.gltf");

    Transform buildTr2(DirectX::XMFLOAT3{ -3.0f,0.45f,3.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.8f,0.8f,0.8f });
    auto pauseActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pause>("pauseActor", buildTr2);

    cameraManager->SetDebugCamera(debugCameraActor);
}

bool MorphScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void MorphScene::DrawGui()
{
    SceneBase::DrawGui();
}
