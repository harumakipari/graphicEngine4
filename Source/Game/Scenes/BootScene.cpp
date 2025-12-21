#include "pch.h"
#include "BootScene.h"

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
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/Cloth.h"
#include "Game/SofyBody/MassPoint.h"

#include "Widgets/ObjectManager.h"
#include "Widgets/Utils/EditorGUI.h"
#include "Widgets/Events/EventSystem.h"
#include "Widgets/TitleUIFactory.h"

#include "Physics/Physics.h"

#include "Graphics/PostProcess/BloomEffect.h"


bool BootScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{

    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    // テストPBD
    {
        pbd = std::make_unique<PBD::System>();
        float stiffness = 0.5f;
#if 0
        // 二点の確認
        pbd->AddParticle({ 0,10,0 }, 0.0f);
        pbd->AddParticle({ 0,3,0 }, 1.0f);
        pbd->AddDistanceConstraints(0, 1, 0.005f);
#else
#if 0
        //三角形二枚の確認
        pbd->AddParticle({ 0,5,0 }, 1.0f);  //0
        pbd->AddParticle({ 2,1,0 }, 1.0f);  //1
        pbd->AddParticle({ -2,1,0 }, 1.0f); //2
        pbd->AddParticle({ 0,1,2 }, 1.0f);  //3

        pbd->AddDistanceConstraints(0, 1, 0.5f);
        pbd->AddDistanceConstraints(1, 2, 0.5f);
        pbd->AddDistanceConstraints(0, 1, 0.5f);
        pbd->AddDistanceConstraints(0, 3, 0.5f);
        pbd->AddDistanceConstraints(1, 3, 0.5f);

        // 曲げ拘束（p1-p2 が共有辺の2三角形で構成）
        pbd->AddBendingConstraint(0, 1, 2, 3, 0.5f);
#else
#if 1
        // グリッドの布
        const int width = 3;
        const int height = 3;
        const float spacing = 1.0f;

        // パーティクル生成
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                //XMFLOAT3 pos = { x * spacing, y * spacing, 0.0f }; // 上空に配置
                XMFLOAT3 pos = { x * spacing, 15.0f, y * spacing }; // 上空に配置
                pbd->AddParticle(pos, 1.0f);
            }
        }


        // グリッド上のインデックスを取得する関数
        auto idx = [&](int x, int y)
            {
                return y * width + x;
            };

        // 上辺パーティクルを固定
        for (int x = 0; x < width; ++x)
        {
            int topIdx = idx(x, 0); // y=0 が上辺
            pbd->GetParticles()[topIdx].invMass = 0.0f;
        }


        // 距離拘束（横・縦・斜め）
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (x + 1 < width)
                    pbd->AddDistanceConstraints(idx(x, y), idx(x + 1, y), stiffness);

                if (y + 1 < height)
                    pbd->AddDistanceConstraints(idx(x, y), idx(x, y + 1), stiffness);

                // 斜め（クロス） これバラバラになる
#if 0
                if (x + 1 < width && y + 1 < height)
                    pbd->AddDistanceConstraints(idx(x, y), idx(x + 1, y + 1), stiffness);
                if (x - 1 >= 0 && y + 1 < height)
                    pbd->AddDistanceConstraints(idx(x, y), idx(x - 1, y + 1), stiffness);

#endif // 0
            }
        }
        // 三角形面構築（Figure 4対応：隣接面間にBending）
        for (int y = 0; y < height - 1; ++y)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                int p0 = idx(x, y); int p1 = idx(x + 1, y); int p2 = idx(x, y + 1); int p3 = idx(x + 1, y + 1); // 三角形2枚（左上・右下） //
                pbd->AddBendingConstraint(p0, p1, p2, p3, stiffness);
                pbd->AddBendingConstraint(p0, p2, p1, p3, stiffness);
            }
        }


        // 上向き法線 (0,1,0)、地面は y=4 にあるように
        XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
        float offset = 4.0f; // 平面方程式 n・x + d = 0 → y + d = 0 → y=0
        float restitution = 0.8f; // 少し弾ませる
        pbd->AddCollisionPlane(normal, offset, restitution);


#else
#if 0
        // 立方体 2 * 2
        float spacing = 1.0f;
        int N = 2; // 各軸の粒子数
        for (int z = 0; z < N; ++z)
        {
            for (int y = 0; y < N; ++y)
            {
                for (int x = 0; x < N; ++x)
                {
                    XMFLOAT3 pos = { x * spacing, y * spacing + 10.0f, z * spacing }; // 上空に配置
                    pbd->AddParticle(pos, 1.0f);
                }
            }
        }

        auto idx = [&](int x, int y, int z) { return x + N * (y + N * z); };


        //// 上辺パーティクルを固定
        //for (int z = 0; z < N; ++z)
        //{
        //    for (int x = 0; x < N; ++x)
        //    {
        //        int topIdx = idx(x, 1, z); // y=0 が上辺
        //        pbd->GetParticles()[topIdx].invMass = 0.0f;
        //    }
        //}

        for (int z = 0; z < N; ++z)
        {
            for (int y = 0; y < N; ++y)
            {
                for (int x = 0; x < N; ++x)
                {
                    int i = idx(x, y, z);

                    // +X方向
                    if (x + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y, z), stiffness);

                    // +Y方向
                    if (y + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y + 1, z), stiffness);

                    // +Z方向
                    if (z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y, z + 1), stiffness);

                    if (x + 1 < N && y + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y + 1, z), stiffness);
                    if (x + 1 < N && z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y, z + 1), stiffness);
                    if (y + 1 < N && z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y + 1, z + 1), stiffness);
                }
            }
        }

        // 頂点インデックス
        std::vector<int> cubeVerts;
        for (int i = 0; i < N * N * N; ++i)
            cubeVerts.push_back(i);

        // メッシュ三角形
        std::vector<PBD::Triangle> cubeTris = {
            {0,1,2}, {1,3,2}, // +Y面
            {4,6,5}, {5,6,7}, // -Y面
            {0,2,4}, {4,2,6}, // +X面
            {1,5,3}, {5,7,3}, // -X面
            {0,4,1}, {1,4,5}, // +Z面
            {2,3,6}, {3,7,6}  // -Z面
        };

        // Volume constraint追加
        pbd->AddVolumeConstraint(cubeVerts, cubeTris, 1.0f);


        // 上向き法線 (0,1,0)、地面は y=4 にあるように
        XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
        float offset = 4.0f; // 平面方程式 n・x + d = 0 → y + d = 0 → y=0
        float restitution = 0.8f; // 少し弾ませる
        pbd->AddCollisionPlane(normal, offset, restitution);

        //pbd->EnableSelfCollision(0.5f); // spacingの半分程度で設定
#else
        int N = 4;                  // 各軸4粒子 → 64粒子
        float spacing = 0.5f;       // 粒子間隔

        // 1. 粒子生成（上空に配置）
        for (int z = 0; z < N; ++z)
        {
            for (int y = 0; y < N; ++y)
            {
                for (int x = 0; x < N; ++x)
                {
                    XMFLOAT3 pos = { x * spacing, y * spacing + 10.0f, z * spacing };
                    pbd->AddParticle(pos, 1.0f);
                }
            }
        }

        // 2. インデックス関数
        auto idx = [&](int x, int y, int z) { return x + N * (y + N * z); };

        // 3. 距離拘束の追加
        for (int z = 0; z < N; ++z)
        {
            for (int y = 0; y < N; ++y)
            {
                for (int x = 0; x < N; ++x)
                {
                    int i = idx(x, y, z);

                    // +X方向
                    if (x + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y, z), stiffness);

                    // +Y方向
                    if (y + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y + 1, z), stiffness);

                    // +Z方向
                    if (z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y, z + 1), stiffness);

                    // 斜め（オプション）
                    if (x + 1 < N && y + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y + 1, z), stiffness);
                    if (x + 1 < N && z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x + 1, y, z + 1), stiffness);
                    if (y + 1 < N && z + 1 < N)
                        pbd->AddDistanceConstraints(i, idx(x, y + 1, z + 1), stiffness);
                }
            }
        }

        // 4. 体積拘束用の三角形を生成（各小立方体12三角形）
        std::vector<int> cubeVerts;
        for (int i = 0; i < N * N * N; ++i)
            cubeVerts.push_back(i);

        std::vector<PBD::Triangle> cubeTris;
        for (int z = 0; z < N - 1; ++z)
        {
            for (int y = 0; y < N - 1; ++y)
            {
                for (int x = 0; x < N - 1; ++x)
                {
                    int v0 = idx(x, y, z);
                    int v1 = idx(x + 1, y, z);
                    int v2 = idx(x, y + 1, z);
                    int v3 = idx(x + 1, y + 1, z);
                    int v4 = idx(x, y, z + 1);
                    int v5 = idx(x + 1, y, z + 1);
                    int v6 = idx(x, y + 1, z + 1);
                    int v7 = idx(x + 1, y + 1, z + 1);

                    cubeTris.push_back({ v0,v1,v2 }); cubeTris.push_back({ v1,v3,v2 });
                    cubeTris.push_back({ v4,v6,v5 }); cubeTris.push_back({ v5,v6,v7 });
                    cubeTris.push_back({ v0,v2,v4 }); cubeTris.push_back({ v4,v2,v6 });
                    cubeTris.push_back({ v1,v5,v3 }); cubeTris.push_back({ v5,v7,v3 });
                    cubeTris.push_back({ v0,v4,v1 }); cubeTris.push_back({ v1,v4,v5 });
                    cubeTris.push_back({ v2,v3,v6 }); cubeTris.push_back({ v3,v7,v6 });
                }
            }
        }

        // 体積拘束追加
        pbd->AddVolumeConstraint(cubeVerts, cubeTris, 1.0f);

        // 5. 床衝突
        XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };
        float offset = 2.0f;         // y=4 の高さ
        float restitution = 0.8f;    // 弾性
        pbd->AddCollisionPlane(normal, offset, restitution);

        // 6. 自己衝突
        pbd->EnableSelfCollision(spacing * 0.4f); // 粒子間距離の半分
#endif // 0
#endif
#endif
#endif // 1
    }



    //アクターをセット
    SetUpActors();
    EventSystem::Initialize();//追加 UI
    return true;
}

void BootScene::Start()
{
    TitleUIFactory::Create(this);

    auto titleButtonObj = objectManager.FindGameObject("TitleButton");
    auto titleButton = titleButtonObj->GetComponent<Button>();
    titleButton->AddOnClickEvent([&]()
        {
            titlePlayer->OnPushStart();
            mainCameraActor->OnClick();
        });

    auto backButtonObj = objectManager.FindGameObject("BackToTitle");
    auto backButton = backButtonObj->GetComponent<Button>();
    backButton->AddOnClickEvent([&]()
        {
        });

    //タイトルBGM
    UIFactory::SetObjectManager(&objectManager);
    AudioSource* titleBgm = UIFactory::Create("TitleBGM")->AddComponent<AudioSource>(L"./Data/Sound/BGM/title.wav");
    titleBgm->SetVolume(0.5f);
    titleBgm->Play(XAUDIO2_LOOP_INFINITE);



    //uiRoot.root = std::make_unique<UIWidget>("canvas");

    //auto button = std::make_unique<UIButton>("button");
    //button->SetSprite(L"./Data/Textures/UI/start_button.png");
    //button->SetPosition(100, 100);
    //button->SetSize(100, 100);
    //button->onClick = ([&]()
    //    {
    //        button->SetSize(200, 200);
    //    });

    //uiRoot.root->AddChild(std::move(button));

}

void BootScene::Update(float deltaTime)
{
    using namespace DirectX;

    SceneBase::Update(deltaTime);

    //pbd->Update(deltaTime);
    pbd->Update(1 / 60.0f);

    const auto& p = pbd->GetParticles()[0];
    const auto& p1 = pbd->GetParticles()[1];
#ifdef USE_IMGUI
    ImGui::Begin("pbd");
    ImGui::Text("pbdPos0.y:%f", p.position.y);
    ImGui::Text("pbdPos1.y:%f", p1.position.y);
    ImGui::End();
#endif

    float mousePosX = static_cast<float>(InputSystem::GetMousePositionX());
    float mousePosY = static_cast<float>(InputSystem::GetMousePositionY());

    auto build = GetActorManager()->GetActorByName("elasticBuilding");
    mainCameraActor->SetTarget(build->GetPosition());
    //uiRoot.OnClick(mousePosX, mousePosY);

    //ActorManager::Update(deltaTime);
    Physics::Instance().Update(deltaTime);
    EventSystem::Update(deltaTime);//追加
    objectManager.Update(deltaTime);//追加

#ifdef _DEBUG
    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
    }
#endif // !_DEBUG

}

void BootScene::SetUpActors()
{
    mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();

    Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,-6.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    titlePlayer = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitlePlayer>("actor", playerTr);

    Transform titleTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto titleStage = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleStage>("title", titleTr);

    Transform buildTr(DirectX::XMFLOAT3{ 4.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto elasticBuilding = this->GetActorManager()->CreateAndRegisterActorWithTransform<ElasticBuilding>("elasticBuilding", buildTr);

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

#if 0
    CameraManager::SetGameCamera(mainCameraActor.get());
#else
    CameraManager::SetGameCamera(debugCameraActor.get());
#endif // 0
    stageCollisionMesh = std::make_shared<CollisionMesh>(Graphics::GetDevice(), "./Data/Models/Stage/stage.gltf", true);

    Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-15.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<EmptyEnemy>("enemy", enemyTr);


    CameraManager::SetDebugCamera(debugCameraActor);
}

bool BootScene::Uninitialize(ID3D11Device* device)
{
    Physics::Instance().Finalize();
    return true;
}

void BootScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    SceneBase::Render(immediateContext, deltaTime);
}





void BootScene::DrawGui()
{
    pbd->DrawGui();
    SceneBase::DrawGui();
}
#if 0
#ifdef USE_IMGUI

    // ==== UIアウトライナ(追加)　====
    EditorGUI::DrawMainMenu();

    if (ImGui::TreeNodeEx("UI Outliner", ImGuiTreeNodeFlags_DefaultOpen))
    {
        objectManager.DrawHierarchy();

        ImGui::TreePop();
    }


    ImGui::End();




#if 0
    // ========== Volumetric Cloudscapes ==========
    if (ImGui::Checkbox("Enable Volumetric Clouds", &enableVolumetricClouds)) {}
    if (enableVolumetricClouds && ImGui::TreeNode("Cloud Settings")) {
        ImGui::DragFloat4("Camera Focus", &cameraFocus.x, 0.5f);

        ImGui::DragFloat("Density Scale", &volumetricCloudscapes->constantData.densityScale, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Cloud Coverage", &volumetricCloudscapes->constantData.cloudCoverageScale, 0.001f, 0.0f, 0.5f);
        ImGui::DragFloat("Rain Absorption", &volumetricCloudscapes->constantData.rainCloudAbsorptionScale, 0.01f, 0.0f, 10.0f, "%.2f");
        ImGui::DragFloat("Cloud Type", &volumetricCloudscapes->constantData.cloudTypeScale, 0.01f, 0.0f, 10.0f, "%.2f");

        ImGui::DragFloat("LowFreq Perlin", &volumetricCloudscapes->constantData.lowFrequencyPerlinWorleySamplingScale, 0.000001f, 0.0f, 1.0f, "%.7f");
        ImGui::DragFloat("HighFreq Worley", &volumetricCloudscapes->constantData.highFrequencyWorleySamplingScale, 0.00001f, 0.0f, 1.0f, "%.5f");
        ImGui::DragFloat("Horizon Distance", &volumetricCloudscapes->constantData.horizonDistanceScale, 0.0001f, 0.0f, 1.0f, "%.4f");

        ImGui::SliderFloat2("Wind Direction", &volumetricCloudscapes->constantData.windDirection.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Wind Speed", &volumetricCloudscapes->constantData.windSpeed, 0.0f, 20.0f);

        ImGui::DragFloat("Earth Radius", &volumetricCloudscapes->constantData.earthRadius, 1.0f);
        ImGui::DragFloat("Cloud Altitude Min", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.x, 1.0f);
        ImGui::DragFloat("Cloud Altitude Max", &volumetricCloudscapes->constantData.cloudAltitudesMinMax.y, 1.0f);

        ImGui::DragFloat("Long Distance Density", &volumetricCloudscapes->constantData.cloudDensityLongDistanceScale, 0.01f, 0.0f, 36.0f, "%.2f");
        ImGui::Checkbox("Powdered Sugar Effect", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.enablePowderedSugarEffect));

        ImGui::SliderInt("Ray Marching Steps", &volumetricCloudscapes->constantData.rayMarchingSteps, 1, 128);
        ImGui::Checkbox("Auto Ray Marching", reinterpret_cast<bool*>(&volumetricCloudscapes->constantData.autoRayMarchingSteps));

        ImGui::TreePop();
    }
#endif
    ImGui::EndTabItem();


// ==== UIタブ（追加） ====
if (ImGui::BeginTabItem("UI"))
{
    objectManager.DrawProperty();

    ImGui::Separator();
    ImGui::Text("EventSystem");
    ImGui::Separator();

    EventSystem::DrawProperty();

    ImGui::EndTabItem();
}
ImGui::EndTabBar();

ImGui::End();

#endif  
#endif // 0