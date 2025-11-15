#include "BootScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
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
#include "Graphics/PostProcess/FogEffect.h"
#include "Graphics/PostProcess/SSAOEffect.h"
#include "Graphics/PostProcess/SSREffect.h"


bool BootScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
#if 0
    sceneCBuffer = std::make_unique<ConstantBuffer<FrameConstants>>(device);
    shaderCBuffer = std::make_unique<ConstantBuffer<ShaderConstants>>(device);
    sceneCBuffer->data.elapsedTime = 0;//開始時に０にしておく

    // ライト
    {
        lightManager = std::make_unique<LightManager>();
        lightManager->Initialize(device);
        lightManager->SetDirectionalLight(lightDirection, lightColor);
    }

    // ポストエフェクト
    {
        postEffectManager = std::make_unique<PostEffectManager>();
        postEffectManager->AddEffect(std::make_unique<BloomEffect>());
        postEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
    }

    // シーンエフェクト
    {
        sceneEffectManager = std::make_unique<SceneEffectManager>();
        sceneEffectManager->AddEffect(std::make_unique<FogEffect>());
        sceneEffectManager->AddEffect(std::make_unique<SSAOEffect>());
        sceneEffectManager->AddEffect(std::make_unique<SSREffect>());
        sceneEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
    }
    HRESULT hr = { S_OK };

    //スカイマップ
    skyMap = std::make_unique<decltype(skyMap)::element_type>(device, L"./Data/Environment/Sky/cloud/skybox.dds");

    fullscreenQuadTransfer = std::make_unique<FullScreenQuad>(device);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, static_cast<uint32_t>(width), height, 3);

    // GBUFFER
    gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
    hr = CreatePsFromCSO(device, "./Shader/DeferredPS.cso", deferredPs.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    hr = CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", finalPs.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //CascadedShadowMaps
    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    D3D11_TEXTURE2D_DESC texture2dDesc;
    //テクスチャをロード
    hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_charlie.dds", environmentTextures[0].ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/diffuse_iem.dds", environmentTextures[1].ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/specular_pmrem.dds", environmentTextures[2].ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured/lut_sheen_e.dds", environmentTextures[3].ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif // 0


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

        pbd->AddParticle({ 0,5,0 }, 0.0f);  //0
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
#if 0
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
    //TitleUIFactory::Create(this);

    //auto titleButtonObj = objectManager.FindGameObject("TitleButton");
    //auto titleButton = titleButtonObj->GetComponent<Button>();
    //titleButton->AddOnClickEvent([&]()
    //    {
    //        //titlePlayer->PlayAnimation("Rotation", false);
    //        titlePlayer->OnPushStart();
    //        mainCameraActor->OnClick();
    //    });

    //auto backButtonObj = objectManager.FindGameObject("BackToTitle");
    //auto backButton = backButtonObj->GetComponent<Button>();
    //backButton->AddOnClickEvent([&]()
    //    {
    //        titlePlayer->OnPushBackToTitle();
    //        title->OnPushBackToTitle();
    //    });

    ////タイトルBGM
    //UIFactory::SetObjectManager(&objectManager);
    //AudioSource* titleBgm = UIFactory::Create("TitleBGM")->AddComponent<AudioSource>(L"./Data/Sound/BGM/title.wav");
    //titleBgm->SetVolume(0.5f);
    //titleBgm->Play(XAUDIO2_LOOP_INFINITE);



    uiRoot.root = std::make_unique<UIWidget>("canvas");

    auto button = std::make_unique<UIButton>("button");
    button->SetSprite(L"./Data/Textures/UI/start_button.png");
    button->SetPosition(100, 100);
    button->SetSize(100, 100);
    button->onClick = ([&]()
        {
            button->SetSize(200, 200);
        });

    uiRoot.root->AddChild(std::move(button));

}

void BootScene::Update(float deltaTime)
{
    using namespace DirectX;

    SceneBase::Update(deltaTime);

    //lightManager->Update(deltaTime);

    pbd->Update(deltaTime);
    //pbd->Update(1 / 60.0f);

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


    //if (InputSystem::GetInputState("F8", InputStateMask::Trigger))
    //{
    //    CameraManager::ToggleCamera();
    //}
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
    //title = this->GetActorManager()->CreateAndRegisterActor<Stage>("title");
    auto titleStage = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleStage>("title", titleTr);

    Transform buildTr(DirectX::XMFLOAT3{ 4.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto elasticBuilding = this->GetActorManager()->CreateAndRegisterActorWithTransform<ElasticBuilding>("elasticBuilding", buildTr);

    //Transform ballTr(DirectX::XMFLOAT3{ 0.0f,5.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto springyBall = this->GetActorManager()->CreateAndRegisterActorWithTransform<SpringyBall>("springyBall", ballTr);

    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });

    //CameraManager::SetGameCamera(mainCameraActor.get());
    CameraManager::SetGameCamera(debugCameraActor.get());

    stageCollisionMesh = std::make_shared<CollisionMesh>(Graphics::GetDevice(), "./Data/Models/Stage/stage.gltf", true);

    //#if 1
    //    Transform sphereTr(DirectX::XMFLOAT3{ 0.0f,10.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //    auto sphereTest = this->GetActorManager()->CreateAndRegisterActorWithTransform<SphereTest>("sphereTest", sphereTr);
    //
    //#endif // 0
    //
    //Transform planeTr(DirectX::XMFLOAT3{ 0.0f,15.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 0.1f,0.1f,0.1f });
    //auto planeTest = this->GetActorManager()->CreateAndRegisterActorWithTransform<TestPBD>("TestPBD", planeTr);


    //auto debugCameraActor = ActorManager::CreateAndRegisterActor<Actor>("debugCam");
    //auto debugCamera = debugCameraActor->NewSceneComponent<DebugCameraComponent>("debugCamera");

    //Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-35.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    Transform enemyTr(DirectX::XMFLOAT3{ 6.7f,0.0f,5.6f }, DirectX::XMFLOAT3{ 0.0f,-15.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto enemy = this->GetActorManager()->CreateAndRegisterActorWithTransform<EmptyEnemy>("enemy", enemyTr);


    CameraManager::SetDebugCamera(debugCameraActor);
}

//bool BootScene::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
//{
//    framebufferDimensions.cx = static_cast<LONG>(width);
//    framebufferDimensions.cy = static_cast<LONG>(height);
//
//    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);
//
//    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);
//
//    postEffectManager->Initialize(device, framebufferDimensions.cx, framebufferDimensions.cy);
//    sceneEffectManager->Initialize(device, framebufferDimensions.cx, framebufferDimensions.cy);
//    return true;
//}

bool BootScene::Uninitialize(ID3D11Device* device)
{
    //ClearActorManager();
    Physics::Instance().Finalize();
    return true;
}

void BootScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
#if 0
    //サンプラーステートを設定
    RenderState::BindSamplerStates(immediateContext);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    // IBL
    immediateContext->PSSetShaderResources(32, 1, environmentTextures[0].GetAddressOf());
    immediateContext->PSSetShaderResources(33, 1, environmentTextures[1].GetAddressOf());
    immediateContext->PSSetShaderResources(34, 1, environmentTextures[2].GetAddressOf());
    immediateContext->PSSetShaderResources(35, 1, environmentTextures[3].GetAddressOf());

    D3D11_VIEWPORT viewport;
    UINT num_viewports{ 1 };
    immediateContext->RSGetViewports(&num_viewports, &viewport);

    float aspect_ratio{ viewport.Width / viewport.Height };


    // SCREEN_SPACE_AMBIENT_OCCLUSION
    shaderCBuffer->data.enableSsao = enableSSAO;
    shaderCBuffer->data.enableBloom = enableBloom;
    shaderCBuffer->data.enableFog = enableFog;
    shaderCBuffer->data.enableCascadedShadowMaps = enableCascadedShadowMaps;
    shaderCBuffer->data.enableSsr = enableSSR;
    // SCREEN_SPACE_REFLECTION
    //sceneCBuffer->data.reflectionIntensity = reflectionIntensity;
    // FOG
    sceneCBuffer->data.elapsedTime += deltaTime;
    sceneCBuffer->data.deltaTime = deltaTime;

    sceneCBuffer->Activate(immediateContext, 1); // slot1 にセット

    //shaderCBuffer->data.maxDistance = 20;
    //shaderCBuffer->data.resolution = resolution;
    //shaderCBuffer->data.steps = steps;
    //shaderCBuffer->data.thickness = thickness;
    shaderCBuffer->Activate(immediateContext, 9); // slot2 にセット

    // slot3 は cascadedShadowMap に使用中
    //fogCBuffer->Activate(immediateContext, 8); // slot4 にセット

    lightManager->Apply(immediateContext, 11);
#endif // 0

    auto camera = CameraManager::GetCurrentCamera();
    if (camera)
    {
        ViewConstants data = camera->GetViewConstants();
        sceneRender.UpdateViewConstants(immediateContext, data);
    }


    UpdateConstantBuffer(immediateContext);

    if (!useDeferredRendering)
    {// フォワードレンダリング
        // MULTIPLE_RENDER_TARGETS
        multipleRenderTargets->Clear(immediateContext);
        multipleRenderTargets->Activate(immediateContext);

        // SKY_MAP
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //skyMap->Blit(immediateContext, sceneConstants.viewProjection);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //splash->Render(immediateContext, 0, 0, viewport.Width, viewport.Height);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_BACK);

        // MULTIPLE_RENDER_TARGETS
        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        //RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        sceneRender.currentRenderPath = RenderPath::Forward;
        sceneRender.RenderOpaque(immediateContext);
        sceneRender.RenderMask(immediateContext);
        sceneRender.RenderBlend(immediateContext);

        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);

        // デバック描画
        //const auto& p = pbd->GetParticles();
        ////static std::vector<XMFLOAT3> points;
        ////points.emplace_back((p.position));
        ////ShapeRenderer::DrawSegment(immediateContext, { 1,0,1,1 }, points, ShapeRenderer::Type::Point);
        //ShapeRenderer::DrawPoint(immediateContext, p[0].position, { 1,0,1,1 });
        //ShapeRenderer::DrawLineSegment(immediateContext, p[0].position, p[1].position, { 1,0,1,1 });
        //ShapeRenderer::DrawLineSegment(immediateContext, p[1].position, p[2].position, { 1,0,1,1 });
        //ShapeRenderer::DrawLineSegment(immediateContext, p[0].position, p[2].position, { 1,0,1,1 });
        //ShapeRenderer::DrawLineSegment(immediateContext, p[0].position, p[3].position, { 0,1,1,1 });
        //ShapeRenderer::DrawLineSegment(immediateContext, p[1].position, p[3].position, { 0,1,1,1 });

        pbd->DebugRender(immediateContext);

#if _DEBUG
        actorColliderManager.DebugRender(immediateContext);
        //PhysicsTest::Instance().DebugRender(immediateContext);
        //GameManager::DebugRender(immediateContext);
#endif
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);


        multipleRenderTargets->Deactivate(immediateContext);


        DirectX::XMFLOAT4X4 cameraView;
        DirectX::XMFLOAT4X4 cameraProjection;

        if (camera)
        {
            ViewConstants data = camera->GetViewConstants();
            cameraView = data.view;
            cameraProjection = data.projection;
        }
        // CASCADED_SHADOW_MAPS
        // Make cascaded shadow maps
        cascadedShadowMaps->Clear(immediateContext);
        cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightDirection, criticalDepthValue, 3/*cbSlot*/);
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //actorRender.CastShadowRender(immediateContext);
        sceneRender.currentRenderPath = RenderPath::Shadow;
        sceneRender.CastShadowRender(immediateContext);
        //gameWorld_->CastShadowRender(immediateContext);
        cascadedShadowMaps->Deactive(immediateContext);

        // CASCADED_SHADOW_MAPS
        // Draw shadow to scene framebuffer
        // FINAL_PASS
        {
            RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
            RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
            RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

            sceneEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)], multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::NORMAL)],
                multipleRenderTargets->depthStencilShaderResourceView, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::POSITION)], cascadedShadowMaps->depthMap().Get());
            postEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[0]);


            ID3D11ShaderResourceView* shader_resource_views[]
            {
                // MULTIPLE_RENDER_TARGETS
                multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)],  //colorMap
                multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::POSITION)],
                multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::NORMAL)],
                multipleRenderTargets->depthStencilShaderResourceView,      //depthMap
                postEffectManager->GetOutput("BloomEffect"),
                sceneEffectManager->GetOutput("FogEffect"),
                sceneEffectManager->GetOutput("SSAOEffect"),    //SSAO
                sceneEffectManager->GetOutput("SSREffect"),
                cascadedShadowMaps->depthMap().Get(),   //cascadedShadowMaps
            };
            // メインフレームバッファとブルームエフェクトを組み合わせて描画
            fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), finalPs.Get());

        }
    }
    else
    {// ディファードレンダリング
        gBufferRenderTarget->Clear(immediateContext);
        gBufferRenderTarget->Acticate(immediateContext);

        // SKY_MAP
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        if (camera)
        {
            ViewConstants data = camera->GetViewConstants();
            skyMap->Blit(immediateContext, data.viewProjection);
        }
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        sceneRender.currentRenderPath = RenderPath::Deferred;
        sceneRender.RenderOpaque(immediateContext);
        sceneRender.RenderMask(immediateContext);
        //sceneRender.RenderBlend(immediateContext);

        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);

        // デバック描画
#if _DEBUG
        const auto& p = pbd->GetParticles();
        //static std::vector<XMFLOAT3> points;
        //points.emplace_back((p.position));
        //ShapeRenderer::DrawSegment(immediateContext, { 1,0,1,1 }, points, ShapeRenderer::Type::Point);
        ShapeRenderer::DrawPoint(immediateContext, p[0].position, { 1,0,1,1 });
        ShapeRenderer::DrawLineSegment(immediateContext, p[0].position, p[1].position, { 1,0,1,1 });
        actorColliderManager.DebugRender(immediateContext);
        //PhysicsTest::Instance().DebugRender(immediateContext);
        //GameManager::DebugRender(immediateContext);
#endif
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        //actorRender.RenderOpaque(immediateContext);
        //actorRender.RenderMask(immediateContext);
        //actorRender.RenderBlend(immediateContext);

        gBufferRenderTarget->Deactivate(immediateContext);

        // MULTIPLE_RENDER_TARGETS
#if 1
        multipleRenderTargets->Clear(immediateContext);
        multipleRenderTargets->Activate(immediateContext);
#endif
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //splash->Render(immediateContext, 0, 0, viewport.Width, viewport.Height);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        // MULTIPLE_RENDER_TARGETS
        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);


        // ここでライティングの処理
        ID3D11ShaderResourceView* shaderResourceViews[]
        {
            // MULTIPLE_RENDER_TARGETS
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],  // normalMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::PBR_VALUE)],   // msrMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::COLOR)],   // colorMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],   // positionMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::EMISSIVE)],   // emissiveMap
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), deferredPs.Get());

        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);

        sceneRender.currentRenderPath = RenderPath::Forward;
        sceneRender.RenderBlend(immediateContext);


#if 1
        multipleRenderTargets->Deactivate(immediateContext);
#endif

        DirectX::XMFLOAT4X4 cameraView;
        DirectX::XMFLOAT4X4 cameraProjection;

        if (camera)
        {
            ViewConstants data = camera->GetViewConstants();
            cameraView = data.view;
            cameraProjection = data.projection;
#if 0
            cameraView = camera->GetView();
            cameraProjection = camera->GetProjection();

#endif // 0
        }
        // CASCADED_SHADOW_MAPS
        // Make cascaded shadow maps
        cascadedShadowMaps->Clear(immediateContext);
        cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightDirection, criticalDepthValue, 3/*cbSlot*/);
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //actorRender.CastShadowRender(immediateContext);
        sceneRender.currentRenderPath = RenderPath::Shadow;
        sceneRender.CastShadowRender(immediateContext);
        //gameWorld_->CastShadowRender(immediateContext);
        cascadedShadowMaps->Deactive(immediateContext);


        // CASCADED_SHADOW_MAPS
        // Draw shadow to scene framebuffer
        // FINAL_PASS
        {
            RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
            RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
            RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
            postEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)]);
            sceneEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)], gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],
                gBufferRenderTarget->depthStencilShaderResourceView, gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)], cascadedShadowMaps->depthMap().Get());


            ID3D11ShaderResourceView* shader_resource_views[]
            {
                //gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::COLOR)],  //colorMap
                multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)],  //colorMap
                gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],   // positionMap
                gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],   // normalMap
                gBufferRenderTarget->depthStencilShaderResourceView,      //depthMap
                postEffectManager->GetOutput("BloomEffect"),
                sceneEffectManager->GetOutput("FogEffect"),
                sceneEffectManager->GetOutput("SSAOEffect"),
                sceneEffectManager->GetOutput("SSREffect"),
                cascadedShadowMaps->depthMap().Get(),   //cascadedShadowMaps
            };
            // メインフレームバッファとブルームエフェクトを組み合わせて描画
            fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), finalPs.Get());
        }
    }

    // UI描画
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        objectManager.Draw(immediateContext);
        //softBodyEngine.Render(immediateContext);
        //uiRoot.Draw(immediateContext);
    }
}

void BootScene::DrawGui()
{
    pbd->DrawGui();
    SceneBase::DrawGui();
#if 0
    sceneEffectManager->DrawGui();

    postEffectManager->DrawGui();

#ifdef USE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.08f, 0.15f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.1f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.15f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.2f, 0.4f, 0.8f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.15f, 0.25f, 0.9f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.25f, 0.4f, 1.0f);
    // ビューポートサイズ取得
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float screen_width = viewport->WorkSize.x;
    const float screen_height = viewport->WorkSize.y;

    const float left_panel_width = 300.0f;
    const float right_panel_width = 400.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 300.0f,
        viewport->WorkPos.y + viewport->WorkSize.y - 100.0f));
    ImGui::SetNextWindowBgAlpha(0.3f); // 半透明


    // ==== 左側：アウトライナー ====
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(left_panel_width, screen_height));
    ImGui::Begin("Actor Outliner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    for (auto& actor : this->GetActorManager()->GetAllActors()) {
        bool is_selected = (selectedActor_ == actor);
        if (ImGui::Selectable(actor->GetName().c_str(), is_selected)) {
            selectedActor_ = actor;
        }
    }

    // ==== UIアウトライナ(追加)　====
    EditorGUI::DrawMainMenu();

    if (ImGui::TreeNodeEx("UI Outliner", ImGuiTreeNodeFlags_DefaultOpen))
    {
        objectManager.DrawHierarchy();

        ImGui::TreePop();
    }


    ImGui::End();

    // ==== 右側：インスペクタ ====
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + screen_width - right_panel_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(right_panel_width, screen_height));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // タブUI開始
    if (ImGui::BeginTabBar("InspectorTabs"))
    {
        // ===== Actorタブ =====
        if (ImGui::BeginTabItem("Actor"))
        {
            if (selectedActor_) {
                selectedActor_->DrawImGuiInspector();
            }
            else {
                ImGui::Text("No actor selected.");
            }
            ImGui::EndTabItem();
        }

        // ===== PostEffectタブ =====
        if (ImGui::BeginTabItem("PostEffect"))
        {
            // -------------------------
            // SSAO
            // -------------------------
            if (ImGui::Checkbox("Enable SSAO", &enableSSAO))
            {
                // SSAO有効切り替え時の処理があればここで
            }
#if 0
            if (enableSSAO && ImGui::TreeNode("SSAO Settings")) {
                ImGui::SliderFloat("Radius", &ssaoRadius, 0.1f, 10.0f);
                ImGui::SliderInt("Sample Count", &ssaoSamples, 4, 64);
                ImGui::TreePop();
            }
#endif

            // ========== SSR ==========
            if (ImGui::Checkbox("Enable SSR", &enableSSR)) {}
            if (enableSSR && ImGui::TreeNode("SSR Settings"))
            {
                //ImGui::SliderFloat("Reflection Intensity", &reflectionIntensity, 0.0f, 1.0f);
                //ImGui::SliderFloat("Max Distance", &maxDistance, 0.0f, 30.0f);
                //ImGui::SliderFloat("Resolution", &resolution, 0.0f, 1.0f);
                //ImGui::SliderInt("Steps", &steps, 0, 20);
                //ImGui::SliderFloat("Thickness", &thickness, 0.0f, 1.0f);
                ImGui::TreePop();
            }

            // -------------------------
            // Bloom
            // -------------------------
            if (ImGui::Checkbox("Enable Bloom", &enableBloom))
            {
            }
            if (enableBloom && ImGui::TreeNode("Bloom Settings"))
            {
                //ImGui::SliderFloat("Threshold", &bloomer->bloom_extraction_threshold, 0.0f, 5.0f);
                //ImGui::SliderFloat("Intensity", &bloomer->bloom_intensity, 0.0f, 5.0f);
                ImGui::TreePop();
            }

            // -------------------------
            // Fog
            // -------------------------
            if (ImGui::Checkbox("Enable Fog", &enableFog))
            {
            }
            if (enableFog && ImGui::TreeNode("Fog Settings"))
            {
                //ImGui::ColorEdit3("Fog Color", fogCBuffer->data.fogColor);
                //ImGui::SliderFloat("Intensity", &(fogCBuffer->data.fogColor[3]), 0.0f, 10.0f);
                //ImGui::SliderFloat("Density", &fogCBuffer->data.fogDensity, 0.0f, 0.05f, "%.6f");
                //ImGui::SliderFloat("Height Falloff", &fogCBuffer->data.fogHeightFalloff, 0.001f, 1.0f, "%.4f");
                //ImGui::SliderFloat("Cutoff Distance", &fogCBuffer->data.fogCutoffDistance, 0.0f, 1000.0f);
                //ImGui::SliderFloat("Ground Level", &fogCBuffer->data.groundLevel, -100.0f, 100.0f);
                //ImGui::SliderFloat("Mie Scattering", &fogCBuffer->data.mieScatteringFactor, 0.0f, 1.0f, "%.4f");
                //ImGui::SliderFloat("Time Scale", &fogCBuffer->data.timeScale, 0.0f, 1.0f, "%.4f");
                //ImGui::SliderFloat("Noise Scale", &fogCBuffer->data.noiseScale, 0.0f, 0.5f, "%.4f");
                ImGui::TreePop();
            }

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
        }

        // ===== SceneSettingsタブ（任意）=====
        if (ImGui::BeginTabItem("Scene"))
        {
            // -------------------------
            // Light Settings
            // -------------------------
            if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("useDeferredRendering", &useDeferredRendering);
                lightManager->DrawGUI();
            }

            // -------------------------
            // CSM (シャドウ関連)
            // -------------------------
            if (ImGui::CollapsingHeader("Cascaded Shadow Maps"))
            {
                ImGui::SliderFloat("Critical Depth", &criticalDepthValue, 0.0f, 1000.0f);
                ImGui::SliderFloat("Split Scheme", &cascadedShadowMaps->splitSchemeWeight, 0.0f, 1.0f);
                ImGui::SliderFloat("Z Mult", &cascadedShadowMaps->zMult, 1.0f, 100.0f);
                ImGui::Checkbox("Fit To Cascade", &cascadedShadowMaps->fitToCascade);
                ImGui::SliderFloat("Shadow Color", &shaderCBuffer->data.shadowColor, 0.0f, 1.0f);
                ImGui::DragFloat("Depth Bias", &shaderCBuffer->data.shadowDepthBias, 0.00001f, 0.0f, 0.01f, "%.8f");
                bool colorize = shaderCBuffer->data.colorizeCascadedLayer != 0;
                if (ImGui::Checkbox("Colorize Layer", &colorize))
                {
                    shaderCBuffer->data.colorizeCascadedLayer = colorize ? 1 : 0;
                }

                //ImGui::Checkbox("Colorize Layer", &shaderCBuffer->data.colorizeCascadedLayer);
                //ImGui::DragInt("Colorize Layer", &shaderCBuffer->data.colorizeCascadedlayer);
                //ImGui::SliderFloat("Shadow Color", &shaderConstants.shadowColor, 0.0f, 1.0f);
                //ImGui::DragFloat("Depth Bias", &shaderConstants.shadowDepthBias, 0.00001f, 0.0f, 0.01f, "%.8f");
                //ImGui::Checkbox("Colorize Layer", &shaderConstants.colorizeCascadedlayer);
            }
            ImGui::EndTabItem();
        }

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
    }

    ImGui::End();

    ImVec2 padding(10.0f, 10.0f);
    ImVec2 window_pos = ImVec2(viewport->WorkPos.x + padding.x,
        viewport->WorkPos.y + viewport->WorkSize.y - 100.0f);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.25f); // 透明度（0.0f ～ 1.0f）

    ImGui::Begin("ShortcutInfo", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoDecoration);

    ImGui::Text(" ShortcutInfo:");
    ImGui::BulletText("Alt + Enter  : fullscreen");
    ImGui::BulletText("F8           : debugCamera");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#if 0
    ImGui::Text("Video memory usage %d MB", video_memory_usage());
#endif
    ImGui::End();
#endif  
#endif // 0

}
