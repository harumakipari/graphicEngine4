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
#include "Game/OdenGame/OdenCameraTargetActor.h"
#include "Game/OdenGame/OdenHighScoreData.h"
#include "Game/OdenGame/OdenActors/OdenTitleStageActor.h"

#include "Physics/Physics.h"

#include "Physics/CollisionSystem.h"

bool TitleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    //アクターをセット
    SetUpActors();

#if 1
    // 暖簾のモデルを作成
    for (int i = 0; i < 5; i++)
    {
        std::string filename = "./Data/Models/Oden_Title_Stage/Oden_Cloth_Noren_" + std::to_string(i + 1) + ".gltf";
        //std::string filename = "./Data/Models/Oden_Title_Stage/Oden_Cloth_Noren_" + std::to_string(1) + ".gltf";
        clothSimulate[i] = std::make_unique<ClothSimulate>(device, filename);
    }

    clothSimulate[0]->windPhaseOffset = 5.0f;
    clothSimulate[1]->windPhaseOffset = 5.0f;
    clothSimulate[2]->windPhaseOffset = 5.0f;
    clothSimulate[3]->windPhaseOffset = 5.0f;
    clothSimulate[4]->windPhaseOffset = 5.0f;

    clothSimulate[0]->windBase = -0.5f;
    clothSimulate[1]->windBase = 7.0f;
    clothSimulate[2]->windBase = 7.0f;
    clothSimulate[3]->windBase = 6.0f;
    clothSimulate[4]->windBase = -10.5f;

    clothSimulate[0]->windEmitPosition = { -9.5f,10.0f,27.5f };
    clothSimulate[1]->windEmitPosition = { -4.5f,14.0f,25.5f };
    clothSimulate[2]->windEmitPosition = { -34.5f,14.5f,5.5f };
    clothSimulate[3]->windEmitPosition = { -38.0f,4.0f,4.0f };
    clothSimulate[4]->windEmitPosition = { -34.0f,4.0f,5.5f };

    // ここで布を描画する
    RegisterRenderHook(RenderPass::Mask, [&](ID3D11DeviceContext* immediateContext)
        {
            for (int i = 0; i < 5; i++)
            {
                std::string clothName = "noren_" + std::to_string(i + 1);
                if (const auto cloth = GetActorManager()->GetActorByName(clothName))
                {
                    clothSimulate[i]->Render(immediateContext, cloth->GetWorldTransform());
                }
            }
        });

#endif // 0
    // セーブデータをロードする
    OdenHighScoreData::Instance().Load();
    return true;
}

void TitleScene::Start()
{
    // メインカメラのターゲットアクターを生成
    Transform cameraTargetTr(DirectX::XMFLOAT3{ -12.3f,13.8f,-12.5f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    mainCameraTarget = GetActorManager()->CreateAndRegisterActorWithTransform<OdenCameraTargetActor>("MainCameraActorTarget", cameraTargetTr);
    mainCameraTarget->onMoveFinished = [this]()
        {
            if (phase == TitlePhase::CameraMovingIn)
                phase = TitlePhase::DifficultySelect;
            else if (phase == TitlePhase::CameraMovingOut)
                phase = TitlePhase::StartWait;
        };

    // メインカメラアクターを生成
    mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->target = (mainCameraTarget->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(11.0f);
    mainCameraComponent->yaw = DirectX::XMConvertToRadians(16.5f);
    mainCameraComponent->distance = 14.3f;
    //mainCameraComponent->pitch = DirectX::XMConvertToRadians(18.0f);
    //mainCameraComponent->yaw = DirectX::XMConvertToRadians(14.5f);
    //mainCameraComponent->distance = 18.4f;
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("MainSceneのカメラ設定される。"));



    // タイトルのBGM再生
    {
        auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
        auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
        // audioComp->SetSource(L"./Data/Sound/BGM/title.wav");// 屋台音
        audioComp->SetSource(L"./Data/Sound/BGM/A Snowless Winter_1h.wav");// 
        audioComp->SetLoop(true);
        audioComp->SetVolume(0.5f);
        audioComp->Play();
    }

    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();

#if 1
    // スタート簡単ボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/easy.png", "button");
        button->SetWorldPosition({ 300, 50 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"ボタンButton Clicked!");
                const char* types[] = { "0", "1" };
             //   Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","0"} });
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","0"} });

                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    // スタート普通ボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/normal.png", "button");
        button->SetWorldPosition({ 600, 50 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"ボタンButton Clicked!");
                const char* types[] = { "0", "1" };
                Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"),{"difficulty","1"} });
                //SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","1"} });
                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    // スタート難しいボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/difficult.png", "button");
        button->SetWorldPosition({ 900, 50 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"難しいButton Clicked!");
                const char* types[] = { "0", "1" };
                //Scene::_transition("LoadingScene", { std::make_pair("preload", "SampleScene"), std::make_pair("type", types[rand() % 2]) });
                //SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"),{"difficulty","2"} });
                Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"),  {"difficulty","2"} });

                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    // スタートチュートリアルボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/tutorial_button.png", "tutorial_button");
        button->SetWorldPosition({ 300, 250 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"tutorial_button Clicked!");
                const char* types[] = { "0", "1" };
                Scene::_transition("LoadingScene", { std::make_pair("preload", "TutorialScene"),  {"difficulty","1"} });
                //SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TutorialScene"), {"difficulty","1"} });

                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

#endif // 1

    // ゲームスタートボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/start_button.png", "start_button");
        button->SetWorldPosition({ 300, 550 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = [this]()
            {
                Logger::Log(u8"難易度選択のカメラ遷移");
                phase = TitlePhase::CameraMovingIn;
                mainCameraTarget->PlayToTarget(3.0f);

                auto cam = mainCameraActor->GetComponent<TPSCameraComponent>();
                cam->PlayDistance(cam->distance, 19.8f, 3.0f);

                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    // ゲーム戻るボタンの作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/UI/back_to_title.png", "back_to_title");
        button->SetWorldPosition({ 300, 750 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = [this]()
            {
                Logger::Log(u8"カメラを元に戻す");
                phase = TitlePhase::CameraMovingOut;
                mainCameraTarget->PlayToOrigin(3.0f);

                auto cam = mainCameraActor->GetComponent<TPSCameraComponent>();
                cam->PlayDistance(cam->distance, 14.3f, 3.0f);


                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

}

void TitleScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);


    for (int i = 0; i < 5; i++)
    {
        clothSimulate[i]->Update(deltaTime);
    }
    Physics::Instance().Update(Time::UnscaledDeltaTime());
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

}

void TitleScene::SetUpActors()
{
#ifdef _DEBUG
    // デバックカメラアクターを生成
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });
    cameraManager->SetDebugCamera(debugCameraActor);
#endif // !_DEBUG
    // 暖簾を生成
    //Transform clothTr(DirectX::XMFLOAT3{ 6.3f,7.9f,16.5f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });

    // ステージアクターを生成
    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.5f,0.5f,0.5f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenTitleStageActor>("stage", stageTr);


    for (int i = 0; i < 5; i++)
    {
        Transform clothTr(DirectX::XMFLOAT3{ -6.8f + i * 3.1f,15.23f,-6.8f }, DirectX::XMFLOAT3{ 90.0f,90.0f,-90.0f }, DirectX::XMFLOAT3{ -0.46f,1.0f,0.36f });
        //  Transform clothTr(DirectX::XMFLOAT3{ -6.8f + i * 3.2f,15.23f,-6.8f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.48f,1.0f,0.48f });
        std::string actorName = "noren_" + std::to_string(i + 1);
        auto clothActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>(actorName, clothTr);
#if 0
        auto cloth = clothActor->AddComponent<SkeletalMeshComponent>("skeletalMeshComponent");
        cloth->SetModel("./Data/Models/Oden_Title_Stage/Oden_Cloth_Noren_" + std::to_string(1) + ".gltf", true);

#endif // 0
    }


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

