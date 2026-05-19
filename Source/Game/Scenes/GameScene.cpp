#include "pch.h"
#include "GameScene.h"
#include <profiler.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#endif

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"

#include "Game/Actors/Dessert/Pudding.h"
#include "Game/Actors/Dessert/TargetPudding.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/Cloth.h"
#include "Game/ScissorsGame/BobbinActor.h"
#include "Game/ScissorsGame/BossSpawner.h"
#include "Game/ScissorsGame/ButtonBombActor.h"
#include "Game/ScissorsGame/ButtonCoinActor.h"
#include "Game/ScissorsGame/ComboUiActor.h"
#include "Game/ScissorsGame/EnemyBase.h"
#include "Game/ScissorsGame/GameCameraTargetActor.h"
#include "Game/ScissorsGame/GameTimerUIActor.h"
#include "Game/ScissorsGame/ItemHeartActor.h"
#include "Game/ScissorsGame/NeedleEnemyActor.h"
#include "Game/ScissorsGame/RabbitBossEnemy.h"
#include "Game/ScissorsGame/RabbitBossState.h"
#include "Game/ScissorsGame/ScissorsGameManager.h"


#include "Physics/Physics.h"
#include "Game/ScissorsGame/ScissorsPlayer1.h"
#include "Game/ScissorsGame/ScissorsStage.h"
#include "Game/ScissorsGame/ScissorsUiEndActor.h"
#include "Game/ScissorsGame/ScissorsUIStartActor.h"
#include "Game/ScissorsGame/ScoreUiActor.h"
#include "Game/ScissorsGame/WaveManagaer.h"
#include "Game/ScissorsGame/YarnEnemyActor.h"
#include "Game/ScissorsGame/YarnWallActor.h"
#include "Graphics/PostProcess/BloomEffect.h"


#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"


bool GameScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    HRESULT hr = { S_OK };

    lightDirection = { -1.0f,-0.66f,0.6f, 1.0f }; // 上の窓からの光
    lightColor = { 1.0f, 1.0f, 1.0f, 3.6f };
    {
        sceneCBuffer = std::make_unique<ConstantBuffer<FrameConstants>>(device);
        shaderCBuffer = std::make_unique<ConstantBuffer<SceneShaderConstants>>(device);
        sceneCBuffer->data.elapsedTime = 0;//開始時に０にしておく

        // ライト
        {
            lightManager = std::make_unique<LightManager>();
            lightManager->Initialize(device);
            lightManager->SetDirectionalLight(this, lightDirection, lightColor);
        }

        {
            {
                Logger::Log(U8("シーンエフェクトを作成しました！"));
                sceneEffectManager = std::make_unique<SceneEffectManager>();
                sceneEffectManager->AddEffect(std::make_unique<BloomEffect>());
                sceneEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
            }
        }

        //スカイマップ
        skyMap = std::make_unique<decltype(skyMap)::element_type>(device, L"./Data/Environment/Sky/sky/skybox.dds");
        fullscreenQuad = std::make_unique<FullScreenQuad>(device);

        frameBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
        sceneFrameBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
        finalBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
        imGuiGizmoBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);

        // GBUFFER
        gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
        hr = CreatePsFromCSO(device, "./Shader/DeferredLightingDefaultPS.cso", deferredPs.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", finalPs.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //カスケードシャドウマップ
        cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024, 1024, 4);

        D3D11_TEXTURE2D_DESC texture2dDesc;
        //テクスチャをロード
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/sky/lut_charlie.dds", environmentTextures[0].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/sky/diffuse_iem.dds", environmentTextures[1].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/sky/specular_pmrem.dds", environmentTextures[2].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/sky/lut_sheen_e.dds", environmentTextures[3].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // UIマネージャーを初期化
        uiManager = std::make_unique<UIManager>();

        // カメラマネージャー作成
        cameraManager = std::make_unique<CameraManager>();

        float screenWidth = static_cast<float>(Graphics::GetScreenWidth());
        float screenHeight = static_cast<float>(Graphics::GetScreenHeight());
        XMFLOAT2 imageSize = { screenWidth,screenHeight };
        XMFLOAT2 imageMin = { 0.0f,0.0f };

        InputSystem::SetViewportRect(
            imageMin.x,
            imageMin.y,
            imageSize.x,
            imageSize.y
        );
        Graphics::SetViewport(
            imageMin.x,
            imageMin.y,
            imageSize.x,
            imageSize.y
        );
        Logger::Log(U8("UI Render viewport ") + std::to_string(imageMin.x) + std::to_string(imageMin.y) + std::to_string(imageSize.x) + std::to_string(imageSize.y));
    }
    {
        Physics::Instance().Initialize();
    }
    {
        SetUpActors();
    }

    // マウスパ ー
    XMFLOAT2 mouseSize = { 100.0f,100.0f };

    mouseCursorPar = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/mousecursor_pa.png", "mousecursor_pa");
    mouseCursorPar->SetSize(mouseSize);
    mouseCursorPar->SetPivot({ 0.6f, 0.5f });
    mouseCursorPar->SetVisible(false);
    // マウス掴み
    mouseCursorGrab = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/mousecursor_gu.png", "mousecursor_gu");
    mouseCursorGrab->SetSize(mouseSize);
    mouseCursorGrab->SetPivot({ 0.6f, 0.5f });
    mouseCursorGrab->SetVisible(false);

    // マウス　ポーズ
    mouseCursorPause = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/mousecursor_pose.png", "mousecursor_pose");
    mouseCursorPause->SetSize(mouseSize);
    mouseCursorPause->SetPivot({ 0.1f, 0.1f });
    mouseCursorPause->SetVisible(false);

    // デカール用関連
    {
        //	描画用プリミティブ生成
        decal_cube = std::make_unique<GeometricCube>();
        //	デカールテクスチャ読み込み
        //D3D11_TEXTURE2D_DESC texture2d_desc;
        decal_textures.resize(1);
        //hr = LoadTextureFromFile(device, L"./Data/Textures/Decal/gun_holes.png", decal_textures[0].color_shader_resource_view.GetAddressOf(), &texture2d_desc);
        //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        //	事前に置いておく
        decal_datas.resize(1);

        decal_datas[0].translation = { 12.0f,0.0f,12.0f };
        decal_datas[0].rotation.x = DirectX::XM_PIDIV2;
        decal_datas[0].scaling = { 10, 10, 10 };
        decal_datas[0].decal_index = 0;

#if 0
        // 定数バッファを生成する
        decalCBuffer = std::make_shared<ConstantBuffer<gbuffer_decal_constants>>(device);
#endif // 0

        // デカール用ピクセルシェーダーを生成する
        hr = CreatePsFromCSO(device, "./Shader/GBufferDecalPrimitivePS.cso", gbuffer_decal_pixel_shader.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    }

    // ジオメトリックシェーダー
    {
        //	ジオメトリプリミティブ用シェーダー準備
        {
            D3D11_INPUT_ELEMENT_DESC input_element_desc[]
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            hr = CreateVsFromCSO(device, "./Shader/geometricPrimitiveVS.cso", geometric_primitive_vertex_shader.GetAddressOf(), geometric_primitive_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
            _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        }

    }

    normalCoin = {
    0.9f, 2.3f,
    0.15f, 15.0f,
    10, 40.0f, 500.0f,
    "./Data/TeamModels/Item/NormalButtonCoin.glb" };

    bonusCoin = {
    1.0f, 3.5f,
    0.45f, 30.0f,
    25, 200.0f, 800.0f,
    "./Data/TeamModels/Item/BonusButtonCoin.glb" };

    // スコアシステムの初期化
    ScoreSystem::Reset();

    // シーン定数バッファを生成する
    gameSceneCBuffer = std::make_shared<ConstantBuffer<GameSceneConstants>>(device);
    // ゲームオーバー用ピクセルシェーダーを生成する
    hr = CreatePsFromCSO(device, "./Shader/FullScreenGameOverPS.cso", gameOverPs.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    return true;
}

void GameScene::Start()
{
    {
        PROFILE_SCOPE("Create Player");
        Transform playerTr(DirectX::XMFLOAT3{ 12.0f,0.0f,3.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,10.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        //Transform playerTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,10.0f }, DirectX::XMFLOAT3{ 0.01f,0.01f,0.01f });
        player = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsPlayer1>("player", playerTr);
    }

    auto& param = SceneTransitionManager::Instance().GetParams();
    currentStageName = STAGE_NAME::FIRST;

    if (param.contains("stage"))
    {
        std::string stageName = param.at("stage");
        currentStageName = StringToStageName(stageName);
    }

    currentStageName = STAGE_NAME::BOSS;

    // 遊ぶステージ名を記録する
    ScoreSystem::RecordStageName(currentStageName);
    // ステージデータをロードする
    LoadStage(currentStageName);

    auto uiStartActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsUIStartActor>("uiStartActor");
    uiStartActor->SetTexture(currentStageName);

    auto uiFinishActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsUiEndActor>("uiEndActor");

    SceneTransitionManager::Instance().SetOnOpeningFinished([this, uiStartActor]()
        {
            if (currentStageName == STAGE_NAME::BOBBIN_FIRST)
            {// ボビンステージはチュートリアルを終わってから時間測定を始めたいから、ここでは呼ばない。
                if (auto gameManagerActor = GetActorManager()->GetActorOfType<ScissorsGameManager>())
                {
                    gameManagerActor->StartGame();
                }
                audioBgmComponent->Play();
                return;
            }

            uiStartActor->PlayReady([this]()
                {
                    OnGameStart();
                    if (auto gameTimerUi = GetActorManager()->GetActorOfType<GameTimerUiActor>())
                    {
                        gameTimerUi->SetTargetTime(currentStageName);
                    }
                });
        });
#if 1
    // シーンのライト設定などを設定する
    SceneSettings settings = {};
    settings.cascadedShadowMapConstants =
    {
        58.624f,
        0.0f,
        true,
        1.0f
    };
    settings.sceneShaderConstants =
    {
        0.75f,
        0.00011f,
        0.005f,
        0.0f,
        -0.028f,
        0.04f,
        -0.01f,
        0.12f,
        4.6f,
        0.0f,
        80.0f,
        1.0f,
        23.0f,
        0,
        1,
        0,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0.0f,
        { 1.0f,1.0f,1.0f },
        0.0f,
    };
    settings.sceneLightSaveData.sceneConstants =
    {
         { 0.59f, -0.63f, 0.66f, 0.85f },
         { 1.0f, 1.0f, 1.0f, 4.17f/*w colorPower*/ },
         0.278f,
         1,
         1,
         40,

         { 1.0f,1.0f,1.0f },
         1.466f,

         { 0.977f,0.71f,0.168f },
         0.0f,

         { 0.422f,0.333f,0.0f },
         0.0f,

         3.0f,
         1.0f,
         0.7f,
         1.8f,

         1.0f,
         0.3f,
         0.78f,
         0.15f,
    };
    this->SetSceneSettings(settings);

#endif // 1



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


    // ダッシュしているかどうか
    bool playerIsDashing = (player->GetStateMachine()->GetStateName() == "Dash");

    // スコアシステムの更新処理
    ScoreSystem::Update(deltaTime, playerIsDashing);

    if (auto boss = GetActorManager()->GetActorOfType<RabbitBossEnemyActor>())
    {
        gameSceneCBuffer->data.morphWeights.x = boss->morphX;
    }

    // 死亡演出
    if (currentStageName == STAGE_NAME::BOSS)
    {
        if (auto boss = GetActorManager()->GetActorOfType<RabbitBossEnemyActor>())
        {
            DirectX::XMFLOAT3 bossPos = boss->GetPosition();

            bossPos.y = 0.5f;

            DirectX::XMFLOAT2 bossUiPos = WorldToUI(bossPos);

            float deathRadius = boss->GetDeathRadius();
            //float deathRadius = gameOverRadius;

            // 2.0f -> 0.25f -> -0.1f
            gameSceneCBuffer->data.radius = deathRadius;
            gameSceneCBuffer->data.screenSize = { Graphics::GetScreenWidth(),Graphics::GetScreenHeight() };

            //bossUiPos.y += gameOverTargetPosY;

            bossUiPos.x /= gameSceneCBuffer->data.screenSize.x;
            bossUiPos.y /= gameSceneCBuffer->data.screenSize.y;

            gameSceneCBuffer->data.playerScreenPosition = bossUiPos;
        }
    }
    else
    {
        DirectX::XMFLOAT3 playerPos = player->GetPosition();
        DirectX::XMFLOAT2 playerUiPos = WorldToUI(playerPos);

        float deathRadius = player->GetDeathRadius();

        // 2.0f -> 0.15f -> -0.1f
        //gameSceneCBuffer->data.radius = gameOverRadius;
        gameSceneCBuffer->data.radius = deathRadius;
        gameSceneCBuffer->data.screenSize = { Graphics::GetScreenWidth(),Graphics::GetScreenHeight() };

        playerUiPos.x /= gameSceneCBuffer->data.screenSize.x;
        playerUiPos.y /= gameSceneCBuffer->data.screenSize.y;

        gameSceneCBuffer->data.playerScreenPosition = playerUiPos;
    }

    // マウスカーソルの更新処理
    {
        DirectX::XMFLOAT2 cursor;
        // ビューポート外だったら、入力しない
        InputSystem::GetMousePositionUI(cursor);

        mouseCursorPar->SetWorldPosition(cursor);
        mouseCursorGrab->SetWorldPosition(cursor);
        mouseCursorPause->SetWorldPosition(cursor);
        // ポーズ中はゲーム入力を一切受け付けない
        if (Scene::GetCurrentScene()->IsPaused())
        {
            mouseCursorPause->SetVisible(true);

            mouseCursorGrab->SetVisible(false);
            mouseCursorPar->SetVisible(false);
        }
        else
        {
            mouseCursorPause->SetVisible(false);
            if (InputSystem::GetInputState("MouseLeft"))
            {
                mouseCursorGrab->SetVisible(true);
                mouseCursorPar->SetVisible(false);
            }
            else
            {
                mouseCursorPar->SetVisible(true);
                mouseCursorGrab->SetVisible(false);
            }
        }
        //　デカールテスト
#if 0
        HitResultWithActor hit = {};
        uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Floor);
        if (CollisionFunction::RaycastFromMouse(cursor, hit, mask))
        {
            ViewConstants viewData = {};
            if (auto camera = cameraManager->GetRenderCamera(this))
            {
                viewData = camera->GetViewConstants();
            }
            else
            {
                Logger::Error(U8("カメラがない"));
            }

            DirectX::XMMATRIX camera_world_matrix;
            camera_world_matrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&viewData.view));
            DirectX::XMVECTOR Scale, Rotation, Translation;
            DirectX::XMMatrixDecompose(&Scale, &Rotation, &Translation, camera_world_matrix);
            DirectX::XMFLOAT4 rotation;
            DirectX::XMStoreFloat4(&rotation, Rotation);

            decal_data data;
            data.scaling.z = 3;	//	適当に引き延ばす
            data.rotation = MathHelper::ConvertQuaternionToEuler(rotation);
            data.translation = hit.hitPoint;
            data.decal_index = 0;
            decal_datas.push_back(data);
        }
#endif // 0
    }

}

// 定数バッファの更新処理をシーンごとにカスタマイズできるようにするための仮想関数
void GameScene::UpdateConstants(ID3D11DeviceContext* immediateContext, float deltaTime)
{

}

void GameScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    RenderState::BindSamplerStates(immediateContext);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);

    // IBL
    immediateContext->PSSetShaderResources(32, 1, environmentTextures[0].GetAddressOf());
    immediateContext->PSSetShaderResources(33, 1, environmentTextures[1].GetAddressOf());
    immediateContext->PSSetShaderResources(34, 1, environmentTextures[2].GetAddressOf());
    immediateContext->PSSetShaderResources(35, 1, environmentTextures[3].GetAddressOf());

    D3D11_VIEWPORT viewport;
    UINT num_viewports{ 1 };
    immediateContext->RSGetViewports(&num_viewports, &viewport);

    // 定数バッファ更新
    {
        auto& shader = Scene::GetCurrentScene()->GetSceneSettings().sceneShaderConstants;

        shaderCBuffer->data.shadowColor = shader.shadowColor;
        shaderCBuffer->data.shadowDepthBias = shader.shadowDepthBias;
        shaderCBuffer->data.slopeBias = shader.slopeBias;
        shaderCBuffer->data.splitU = shader.splitU;

        shaderCBuffer->data.hueShift = shader.hueShift;
        shaderCBuffer->data.saturation = shader.saturation;
        shaderCBuffer->data.brightness = shader.brightness;
        shaderCBuffer->data.contrast = shader.contrast;

        shaderCBuffer->data.focusDistance = shader.focusDistance;
        shaderCBuffer->data.dofNearRange = shader.dofNearRange;
        shaderCBuffer->data.dofRange = shader.dofRange;
        shaderCBuffer->data.dofBlurStrength = shader.dofBlurStrength;

        shaderCBuffer->data.objectIblIntensity = shader.objectIblIntensity;
        //shaderCBuffer->data.renderStep = shader.renderStep; // これはImGuiで
        shaderCBuffer->data.enableToneMapping = shader.enableToneMapping;
        shaderCBuffer->data.enableSsao = shader.enableSsao;

        shaderCBuffer->data.enableCascadedShadowMaps = shader.enableCascadedShadowMaps;
        shaderCBuffer->data.enableSsr = shader.enableSsr;
        shaderCBuffer->data.enableFog = shader.enableFog;
        shaderCBuffer->data.enableBloom = shader.enableBloom;

        shaderCBuffer->data.enableBlur = shader.enableBlur;
        shaderCBuffer->data.enableDof = shader.enableDof;
        shaderCBuffer->data.colorizeCascadedLayer = shader.colorizeCascadedLayer;
        shaderCBuffer->data.toneMappingValue = shader.toneMappingValue;

        shaderCBuffer->data.colorMapRGB = shader.colorMapRGB;
        shaderCBuffer->data.pad3 = shader.pad3;

        sceneCBuffer->Activate(immediateContext, 1);
        shaderCBuffer->Activate(immediateContext, 9);
    }

    // ゲームシーン定数バッファを更新
    gameSceneCBuffer->Activate(immediateContext, 12);

    // シーンからポイントライト集める
    lightManager->CollectPointLightsFromScene(*this);
    lightManager->Apply(immediateContext, 11);

    // カメラのビュー定数を更新
    ViewConstants data = {};
    if (auto camera = cameraManager->GetRenderCamera(this))
    {
        data = camera->GetViewConstants();
        sceneRender.UpdateViewConstants(immediateContext, data);
    }
    else
    {
        Logger::Error(U8("カメラがない"));
    }


#ifdef USE_IMGUI
    imGuiGizmoBuffer->Clear(immediateContext);
    imGuiGizmoBuffer->Activate(immediateContext);
#endif

    // ディファードレンダリング
    gBufferRenderTarget->Clear(immediateContext);
    gBufferRenderTarget->Acticate(immediateContext);

    auto queues = sceneRender.BuildRenderQueues();

    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);
    sceneRender.currentRenderPath = RenderPath::Deferred;
    sceneRender.RenderOpaque(immediateContext, queues.deferredOpaque);
    ExecuteHooks(RenderPass::Opaque, immediateContext);

    sceneRender.RenderMask(immediateContext, queues.deferredMask);
    ExecuteHooks(RenderPass::Mask, immediateContext);


    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
    gBufferRenderTarget->Deactivate(immediateContext);


    GBufferDecalPass(immediateContext);


    DirectX::XMFLOAT4X4 cameraView;
    DirectX::XMFLOAT4X4 cameraProjection;

    cameraView = data.view;
    cameraProjection = data.projection;

    // 影を作る処理
    auto& shadow = Scene::GetCurrentScene()->GetSceneSettings().cascadedShadowMapConstants;

    cascadedShadowMaps->Clear(immediateContext);
    cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightManager->GetLightDirection(), shadow.criticalDepthValue, 3/*cbSlot*/);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);
    sceneRender.currentRenderPath = RenderPath::Shadow;
    sceneRender.CastShadowRender(immediateContext, queues.shadowCasters);
    cascadedShadowMaps->Deactivate(immediateContext);
    // ライティングのパス
    {
        frameBuffer->Clear(immediateContext);
        frameBuffer->Activate(immediateContext);

        // スカイマップを描画
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);
        skyMap->Blit(immediateContext, data.viewProjection);
        ExecuteHooks(RenderPass::Sky, immediateContext);

        //dummyTexture->Draw(immediateContext);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);

        ID3D11ShaderResourceView* shaderResourceViews[]
        {
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],  // normalMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::PBR_VALUE)],   // msrMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::COLOR)],   // colorMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],   // positionMap
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::EMISSIVE)],   // emissiveMap
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), deferredPs.Get());
        frameBuffer->Deactivate(immediateContext);
    }


    ID3D11ShaderResourceView* shaderResourceViews[]
    {
        gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],  // normalMap w:objectType
    };
    // GBufferのobjectTypeを送る
    immediateContext->PSSetShaderResources(25, 1, shaderResourceViews);

    // フォワードレンダリング
    frameBuffer->Activate(immediateContext, gBufferRenderTarget->depthStencilView);

#if 0
    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_FRONT);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend);
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend);
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);
#else
    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON); // 今回だけ透明を書き込む
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_FRONT);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend);
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend);
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);
#endif // 0



    // 軌跡を描画する 今回のゲームで追加
#if 0
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ADD);
        player->RenderTrail(immediateContext);
        if (needleEnemyActor)
        {
            needleEnemyActor->RenderTrail(immediateContext);
        }
    }

#endif // 0

#if 1
    // PARTICLES
    {
        ProfileScopedSection_2(0, "Particles", ImGuiControl::Profiler::Green);

        //深度ステンシルステート設定
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF, 1);
        //ラスタライザ設定
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);

        //定数バッファ更新


        // パーティクル描画
        EffectManager::Render(immediateContext);

        ExecuteHooks(RenderPass::Particle, immediateContext);
    }

#endif // 0

    // デバック描画
#if _DEBUG
    if (useDrawDebug)
    {
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::WIREFRAME_CULL_NONE);
        //Physics::Instance().Render(cameraView, cameraProjection, { lightDirection.x,lightDirection.y,lightDirection.z });
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
        DebugRender::Render(immediateContext);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::WIREFRAME_CULL_NONE);
        DebugRender::WiredRender(immediateContext);
        ExecuteHooks(RenderPass::Debug, immediateContext);
    }
#endif
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);

    frameBuffer->Deactivate(immediateContext);

#if 1
    sceneEffectManager->ApplyAll(immediateContext, frameBuffer->shaderResourceViews[0].Get(), gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],
        gBufferRenderTarget->depthStencilShaderResourceView, gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)], gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::PBR_VALUE)], cascadedShadowMaps->depthMap().Get());

    ID3D11ShaderResourceView* nullSRVs[16] = {};
    immediateContext->PSSetShaderResources(0, 16, nullSRVs);
#endif

    sceneFrameBuffer->Clear(immediateContext);
    sceneFrameBuffer->Activate(immediateContext);

    // FINAL_PASS
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);

        ID3D11ShaderResourceView* shader_resource_views[]
        {
              frameBuffer->shaderResourceViews[0].Get(),//colorMap   こっちライティング済み
              gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],   // positionMap
              gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],   // normalMap
              gBufferRenderTarget->depthStencilShaderResourceView,      //depthMap
              sceneEffectManager->GetOutput("BloomEffect"),
              cascadedShadowMaps->depthMap().Get(),   //cascadedShadowMaps
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), finalPs.Get());
    }

    // UIの描画
    Draw(immediateContext);

    // マウスカーソルの描画
    if (!InputSystem::IsGamepadConnected())
    {// コントローラーが接続されていないときだけマウスカーソル描画
        if (mouseCursorPar->IsVisible())
            mouseCursorPar->Draw(immediateContext);
        if (mouseCursorPause->IsVisible())
            mouseCursorPause->Draw(immediateContext);
        if (mouseCursorGrab->IsVisible())
            mouseCursorGrab->Draw(immediateContext);
#ifndef _DEBUG
        InputSystem::SetCursolVisible(false);
#endif
    }
    else
    {
        InputSystem::SetCursolVisible(true);
    }

    sceneFrameBuffer->Deactivate(immediateContext);


    // GAME_OVER_PASS
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);

        ID3D11ShaderResourceView* shader_resource_views[]
        {
              sceneFrameBuffer->shaderResourceViews[0].Get(),//colorMap  
        };
        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), gameOverPs.Get());
    }

#ifdef USE_IMGUI
    imGuiGizmoBuffer->Deactivate(immediateContext);
#endif
}

void GameScene::SetUpActors()
{
    Transform mainCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<FixedCamera>("fixedCameraActor", mainCameraTr);
    mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(30), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 20.f, 500.0f);

    //Transform cameraTargetTr(DirectX::XMFLOAT3{ 11.792f,10.5f,-9.8f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto cameraTargetActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("cameraTargetActor", cameraTargetTr);
    //mainCameraActor->SetTarget(cameraTargetActor->GetRootComponent());
    Transform cameraTargetTr(DirectX::XMFLOAT3{ 11.792f,10.5f,-9.8f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto cameraTargetActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<GameCameraTargetActor>("cameraTargetActor", cameraTargetTr);
    mainCameraActor->SetTarget(cameraTargetActor->GetRootComponent());

    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->SetPitch(DirectX::XMConvertToRadians(-31.0f));
    mainCameraComponent->SetFov(DirectX::XMConvertToRadians(30.0f));
    mainCameraComponent->distance = 10.9f;

    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("gameシーンのカメラ設定される。"));

    Transform debugCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam", debugCameraTr);
    cameraManager->SetDebugCamera(debugCameraActor);

    Transform cinemaCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto cinemaCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<CinemaCamera>("cinemaCam", cinemaCameraTr);
    cameraManager->SetCinematicCamera(cinemaCameraActor);

    Transform movieCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto movieCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MovieCamera>("movieCam", movieCameraTr);
    cameraManager->SetMovieCamera(movieCameraActor);

    // ステージを生成
    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,180.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.f,1.f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsStage>("stage", stageTr);

    // ポーズアクターを生成
    auto pauseActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pause>("pauseActor");
    pauseActor->SetRetrySceneName("GameScene");

    // スコア表示アクターを生成
    Transform scoreUiTr(DirectX::XMFLOAT3{ 20.0f,11.2f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.f,1.f });
    auto scoreUiActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScoreUiActor>("scoreUiActor", scoreUiTr);

    // コンボ表示アクターを生成
    Transform comboUiTr(DirectX::XMFLOAT3{ 4.256f,9.3f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.f,1.f });
    auto comboUiActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ComboUiActor>("comboUiActor", comboUiTr);

#if 0// ボスを生成　
    Transform bossTr(DirectX::XMFLOAT3{ 10.5f,0.0f,12.7f }, DirectX::XMFLOAT3{ 0.0f,180.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto rabbitBoss = this->GetActorManager()->CreateAndRegisterActorWithTransform<RabbitBossEnemyActor>("boss", bossTr);
#endif // 0// ボスを生成　

#if 0// ハリネズミを生成　
    Transform needleTr(DirectX::XMFLOAT3{ 1.0f,0.0f,5.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    needleEnemyActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<NeedleEnemyActor>("needleEnemy", needleTr);
    needleEnemyActor->SetMoveDirection({ 1.0f,0.0f,1.0f });
#endif // 0// ハリネズミを生成　
#if 1
    Transform coinTr(DirectX::XMFLOAT3{ 10.5f,0.0f,12.7f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto coin = this->GetActorManager()->CreateAndRegisterActorWithTransform<ButtonCoinActor>("coin", coinTr);
#endif // 0

#if 0 
    Transform coinTr(DirectX::XMFLOAT3{ 10.5f,0.0f,12.7f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto coin = this->GetActorManager()->CreateAndRegisterActorWithTransform<EnemyBase>("enemy", coinTr);

    Transform needleTr(DirectX::XMFLOAT3{ 1.0f,0.0f,5.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto Actor = this->GetActorManager()->CreateAndRegisterActorWithTransform<EnemyBase>("enemy", needleTr);

#endif // 0
#if  0

    //Transform bobbinTr(DirectX::XMFLOAT3{ 4.5f,0.0f,19.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto bobbin = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr);
    //bobbin->SetBobbinSize(BobbinActor::BobbinSize::Medium);

    //Transform bobbinTr1(DirectX::XMFLOAT3{ 19.5f,0.0f,4.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto bobbin1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr1);
    //bobbin1->SetBobbinSize(BobbinActor::BobbinSize::Medium);
#endif // 0

    // ゲームマネージャーを生成
    Transform gameManagerTransform(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto gameManagerActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsGameManager>("gameManagerActor", gameManagerTransform);
    gameManagerActor->SetGameInputEnabled(false);

    //Transform timerActorTransform(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto scissorsUiTimeActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<ScissorsUiTimerActor>("timeActor", timerActorTransform);

    //SpawnEnemy({ 10.5f,0,5 }, YarnEnemyType::Static,true);


    morphModel = std::make_unique<MorphModel>(Graphics::GetDevice(), "./Data/TeamModels/Enemy/MorphTearModel.gltf");

    RegisterRenderHook(RenderPass::ForwardBlend, [&](ID3D11DeviceContext* immediateContext)
        {
            if (const auto boss = GetActorManager()->GetActorOfType<RabbitBossEnemyActor>())
            {
                if (boss->tearStart)
                {// 涙開始していたら。
                    DirectX::XMFLOAT4X4 world = boss->bossTearSceneComponent->GetComponentWorldTransform().ToWorldTransform();
                    morphModel->Render(immediateContext, world, {}, MorphModel::RenderPass::All);
                }
            }
        });

}

// デカールパス
void GameScene::GBufferDecalPass(ID3D11DeviceContext* immediateContext)
{
    UINT viewportCount{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
    D3D11_VIEWPORT cachedViewPorts[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cachedRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cachedDepthStencilView;
    viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    immediateContext->RSGetViewports(&viewportCount, cachedViewPorts);
    immediateContext->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(),
        cachedDepthStencilView.ReleaseAndGetAddressOf());

    static constexpr int SceneCBVIndex = 1;
    static constexpr int GBufferSRVIndex = 0;
    static constexpr int SamplerStateIndex[5] = { 0, 1 , 2, 3, 4 };

    //	出力先をGBufferに変更
    {
        auto rtv = gBufferRenderTarget->renderTargetViews[static_cast<int>(SRV_SLOT::COLOR)];

        immediateContext->OMSetRenderTargets(
            1,
            &rtv,
            gBufferRenderTarget->depthStencilView
        );
        //	ステンシルを利用するのでクリア
        immediateContext->ClearDepthStencilView(gBufferRenderTarget->depthStencilView, D3D11_CLEAR_STENCIL, 0.0f, 0);
    }

    // ビューポートの設定
    D3D11_VIEWPORT scene_viewport{};
    scene_viewport.TopLeftX = 0;
    scene_viewport.TopLeftY = 0;

#if _DEBUG
    scene_viewport.Width = 1280;// Graphics::GetScreenWidth();
    scene_viewport.Height = 720;    // Graphics::GetScreenHeight();
#else
    scene_viewport.Width = Graphics::GetScreenWidth();
    scene_viewport.Height = Graphics::GetScreenHeight();
#endif

    scene_viewport.MinDepth = 0.0f;
    scene_viewport.MaxDepth = 1.0f;
    immediateContext->RSSetViewports(1, &scene_viewport);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);



    //	デカール書き込み
    //for (auto& decal : decal_datas)
    for (auto& decal : player->decal_datas)
    {
        //	姿勢生成
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(decal.scaling.x, decal.scaling.y, decal.scaling.z);
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(decal.rotation.x, decal.rotation.y, decal.rotation.z);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(decal.translation.x, decal.translation.y, decal.translation.z);
        DirectX::XMMATRIX World = S * R * T;
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, World);

        DebugRender::DrawBox(decal.translation, decal.scaling, { 1,1,1,1 }, 0, true);

        //	SRVとして他のGBufferを利用するので設定
        ID3D11ShaderResourceView* shader_resource_views[] =
        {
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::EMISSIVE)],	//	カラーには書き込みを行うので適当にダミーを設定
             gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::EMISSIVE)],
             gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::PBR_VALUE)],
            gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],
        };
        immediateContext->PSSetShaderResources(GBufferSRVIndex, _countof(shader_resource_views), shader_resource_views);

        // 必要な情報を設定しておく
#if 0
        {
            static constexpr int GbufferDecalTextureSRVIndex = 30;
            static constexpr int GbufferDecalTextureCBVIndex = 12;
            immediateContext->PSSetShaderResources(GbufferDecalTextureSRVIndex, 1, decal_textures[decal.decal_index].color_shader_resource_view.GetAddressOf());

            gbuffer_decal_constants decal_constant;
            {
                //	ボックスの向きを保存
                DirectX::XMStoreFloat4(&decal_constant.decal_direction, R.r[2]);
                decal_constant.decal_direction.w = 0;

                //	ボックスの空間に変換するための行列
                DirectX::XMMATRIX V = DirectX::XMMatrixInverse(nullptr, World);
                DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(1, 1, 0, 1);
                DirectX::XMStoreFloat4x4(&decal_constant.decal_inverse_transform, V * P);
            }
            decalCBuffer->data = decal_constant;
            decalCBuffer->Activate(immediateContext, GbufferDecalTextureCBVIndex);
        }
#endif // 0


        {
            //	頂点シェーダー等の設定
            immediateContext->VSSetShader(geometric_primitive_vertex_shader.Get(), nullptr, 0);
            immediateContext->IASetInputLayout(geometric_primitive_input_layout.Get());
            immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            //	裏面描画してステンシル値1を書き込む
            {
                RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
                RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::DECAL, 1);

                std::function<void()> callback = ([&]()
                    {
                        immediateContext->PSSetShader(nullptr, nullptr, 0);
                    });
                DirectX::XMFLOAT4 color = { 1, 1, 1, 1 };
                decal_cube->Render(world, color, callback);
            }

            //	表面描画してステンシル値1と比較
            {
                RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_FRONT);
                RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::DECAL, 0);
                std::function<void()> callback = ([&]()
                    {
                        immediateContext->PSSetShader(gbuffer_decal_pixel_shader.Get(), nullptr, 0);
                    });
                decal_cube->Render(world, decal.color, callback);
            }
        }
    }

    immediateContext->RSSetViewports(viewportCount, cachedViewPorts);
    immediateContext->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get());

    cachedRenderTargetView.Reset();
    cachedDepthStencilView.Reset();
}

// ステージをロードする
void GameScene::LoadStage(STAGE_NAME stageId)
{
#if 1
    auto waveManagerActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<WaveManager>("waveManager");
    waveManagerActor->SetWaves(stageId);
#endif // 0

    // ステージギミックの生成
    SpawnStageGimmicks(stageId);

    // BGMのセットアップ
    SetupBGM(stageId);

    // プレイヤーの初期位置セット
    SetupPlayerPos(stageId);

}

// ゲーム開始処理
void GameScene::OnGameStart()
{
    audioBgmComponent->Play();
    if (auto gameManagerActor = GetActorManager()->GetActorOfType<ScissorsGameManager>())
    {
        gameManagerActor->StartGame();
        gameManagerActor->StartTimer();
    }
}

// ステージごとのギミック生成
void GameScene::SpawnStageGimmicks(STAGE_NAME stageId)
{
    if (stageId != STAGE_NAME::TUTORIAL)
    {// チュートリアル以外なら
        // タイマー表示アクターを生成
        Transform timerUiTr(DirectX::XMFLOAT3{ 19.9f,8.7f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.f,1.f });
        auto timerUiActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<GameTimerUiActor>("timerUiActor", timerUiTr);
    }


    switch (stageId)
    {
    case STAGE_NAME::TUTORIAL:
        break;
    case STAGE_NAME::FIRST:

        break;
    case STAGE_NAME::BOBBIN_FIRST:
    {
        Transform bobbinTr(DirectX::XMFLOAT3{ 12.0f,0.0f,12.f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr);
        bobbin->SetBobbinSize(BobbinActor::BobbinSize::Big);
        bobbin->SetBobbinStateCharge();
    }

    break;
    case STAGE_NAME::REFLECT_WALL:
    {
        {
            Transform yarnWallTr(DirectX::XMFLOAT3{ 10.5f,0.0f,2.7f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            yarnWall->SetBehavior(WallBehavior::HideOnce);
            yarnWall->SetTriggerWave(3);
            yarnWall->SetUp();
            //Transform yarnWallTr(DirectX::XMFLOAT3{ 12.0f,0.0f,3.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            //auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            //yarnWall->SetBehavior(WallBehavior::HideOnce);
            //yarnWall->SetTriggerWave(3);
            //yarnWall->SetUp();

            Transform yarnWallTr1(DirectX::XMFLOAT3{ 13.5f,0.0f,2.7f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr1);
            yarnWall1->SetBehavior(WallBehavior::HideOnce);
            yarnWall1->SetTriggerWave(3);
            yarnWall1->SetUp();

            Transform yarnWallTr2(DirectX::XMFLOAT3{ 10.5f,0.0f,21.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            yarnWall2->SetBehavior(WallBehavior::HideOnce);
            yarnWall2->SetTriggerWave(3);
            yarnWall2->SetUp();
            //Transform yarnWallTr2(DirectX::XMFLOAT3{ 12.0f,0.0f,21.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            //auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            //yarnWall2->SetBehavior(WallBehavior::HideOnce);
            //yarnWall2->SetTriggerWave(3);

            Transform yarnWallTr3(DirectX::XMFLOAT3{ 13.5f,0.0f,21.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall3 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr3);
            yarnWall3->SetBehavior(WallBehavior::HideOnce);
            yarnWall3->SetTriggerWave(3);
            yarnWall3->SetUp();

        }


        {
            Transform yarnWallTr(DirectX::XMFLOAT3{ 4.5f,0.0f,6.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            yarnWall->SetBehavior(WallBehavior::HideOnce);
            yarnWall->SetTriggerWave(3);
            yarnWall->SetUp();

            Transform yarnWallTr1(DirectX::XMFLOAT3{ 7.5f,0.0f,6.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr1);
            yarnWall1->SetBehavior(WallBehavior::HideOnce);
            yarnWall1->SetTriggerWave(3);
            yarnWall1->SetUp();

            Transform yarnWallTr2(DirectX::XMFLOAT3{ 4.5f,0.0f,18.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            yarnWall2->SetBehavior(WallBehavior::HideOnce);
            yarnWall2->SetTriggerWave(3);
            yarnWall2->SetUp();

            Transform yarnWallTr3(DirectX::XMFLOAT3{ 7.5f,0.0f,18.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall3 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr3);
            yarnWall3->SetBehavior(WallBehavior::HideOnce);
            yarnWall3->SetTriggerWave(3);
            yarnWall3->SetUp();

            Transform yarnWallTr4(DirectX::XMFLOAT3{ 16.5f,0.0f,6.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall4 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr4);
            yarnWall4->SetBehavior(WallBehavior::HideOnce);
            yarnWall4->SetTriggerWave(3);
            yarnWall4->SetUp();

            Transform yarnWallTr5(DirectX::XMFLOAT3{ 19.5f,0.0f,6.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall5 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr5);
            yarnWall5->SetBehavior(WallBehavior::HideOnce);
            yarnWall5->SetTriggerWave(3);
            yarnWall5->SetUp();

            Transform yarnWallTr6(DirectX::XMFLOAT3{ 16.5f,0.0f,18.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall6 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr6);
            yarnWall6->SetBehavior(WallBehavior::HideOnce);
            yarnWall6->SetTriggerWave(3);
            yarnWall6->SetUp();

            Transform yarnWallTr7(DirectX::XMFLOAT3{ 19.5f,0.0f,18.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall7 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr7);
            yarnWall7->SetBehavior(WallBehavior::HideOnce);
            yarnWall7->SetTriggerWave(3);
            yarnWall7->SetUp();
        }

        {
            Transform yarnWallTr(DirectX::XMFLOAT3{ 6.0f,0.0f,4.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            yarnWall->SetBehavior(WallBehavior::AppearOnce);
            yarnWall->SetTriggerWave(3);
            yarnWall->SetUp();

            Transform yarnWallTr1(DirectX::XMFLOAT3{ 6.0f,0.0f,7.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr1);
            yarnWall1->SetBehavior(WallBehavior::AppearOnce);
            yarnWall1->SetTriggerWave(3);
            yarnWall1->SetUp();

            Transform yarnWallTr2(DirectX::XMFLOAT3{ 18.0f,0.0f,4.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            yarnWall2->SetBehavior(WallBehavior::AppearOnce);
            yarnWall2->SetTriggerWave(3);
            yarnWall2->SetUp();

            Transform yarnWallTr3(DirectX::XMFLOAT3{ 18.0f,0.0f,7.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall3 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr3);
            yarnWall3->SetBehavior(WallBehavior::AppearOnce);
            yarnWall3->SetTriggerWave(3);
            yarnWall3->SetUp();

            Transform yarnWallTr4(DirectX::XMFLOAT3{ 6.0f,0.0f,16.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall4 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr4);
            yarnWall4->SetBehavior(WallBehavior::AppearOnce);
            yarnWall4->SetTriggerWave(3);
            yarnWall4->SetUp();

            Transform yarnWallTr5(DirectX::XMFLOAT3{ 6.0f,0.0f,19.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall5 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr5);
            yarnWall5->SetBehavior(WallBehavior::AppearOnce);
            yarnWall5->SetTriggerWave(3);
            yarnWall5->SetUp();

            Transform yarnWallTr6(DirectX::XMFLOAT3{ 18.0f,0.0f,16.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall6 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr6);
            yarnWall6->SetBehavior(WallBehavior::AppearOnce);
            yarnWall6->SetTriggerWave(3);
            yarnWall6->SetUp();

            Transform yarnWallTr7(DirectX::XMFLOAT3{ 18.0f,0.0f,19.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall7 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr7);
            yarnWall7->SetBehavior(WallBehavior::AppearOnce);
            yarnWall7->SetTriggerWave(3);
            yarnWall7->SetUp();
        }

        {
            Transform yarnWallTr(DirectX::XMFLOAT3{ 2.5f,0.0f,10.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            yarnWall->SetBehavior(WallBehavior::AppearOnce);
            yarnWall->SetTriggerWave(3);
            yarnWall->SetUp();

            Transform yarnWallTr1(DirectX::XMFLOAT3{ 2.5f,0.0f,13.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr1);
            yarnWall1->SetBehavior(WallBehavior::AppearOnce);
            yarnWall1->SetTriggerWave(3);
            yarnWall1->SetUp();

            Transform yarnWallTr2(DirectX::XMFLOAT3{ 21.5f,0.0f,10.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            yarnWall2->SetBehavior(WallBehavior::AppearOnce);
            yarnWall2->SetTriggerWave(3);
            yarnWall2->SetUp();

            Transform yarnWallTr3(DirectX::XMFLOAT3{ 21.5f,0.0f,13.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall3 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr3);
            yarnWall3->SetBehavior(WallBehavior::AppearOnce);
            yarnWall3->SetTriggerWave(3);
            yarnWall3->SetUp();

        }

#if 0
        {
            Transform yarnWallTr(DirectX::XMFLOAT3{ 10.5f,0.0f,12.f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr);
            yarnWall->SetBehavior(WallBehavior::AppearOnce);
            yarnWall->SetTriggerWave(3);
            yarnWall->SetUp();

            Transform yarnWallTr1(DirectX::XMFLOAT3{ 13.5f,0.0f,12.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr1);
            yarnWall1->SetBehavior(WallBehavior::AppearOnce);
            yarnWall1->SetTriggerWave(3);
            yarnWall1->SetUp();

            Transform yarnWallTr2(DirectX::XMFLOAT3{ 12.0f,0.0f,13.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall2 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr2);
            yarnWall2->SetBehavior(WallBehavior::AppearOnce);
            yarnWall2->SetTriggerWave(3);
            yarnWall2->SetUp();

            Transform yarnWallTr3(DirectX::XMFLOAT3{ 12.0f,0.0f,10.5f }, DirectX::XMFLOAT3{ 0.0f,90.0f,0.0f }, DirectX::XMFLOAT3{ 1.775f,1.775f,1.775f });
            auto yarnWall3 = this->GetActorManager()->CreateAndRegisterActorWithTransform<YarnWallActor>("YarnWallActor", yarnWallTr3);
            yarnWall3->SetBehavior(WallBehavior::AppearOnce);
            yarnWall3->SetTriggerWave(3);
            yarnWall3->SetUp();
        }
#endif // 0
    }
    break;
    case STAGE_NAME::BOBBIN_SECOND:
    {
        Transform bobbinTr(DirectX::XMFLOAT3{ 4.5f,0.0f,4.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr);
        bobbin->SetBobbinSize(BobbinActor::BobbinSize::Medium);

        Transform bobbinTr1(DirectX::XMFLOAT3{ 19.5f,0.0f,19.5f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr1);
        bobbin1->SetBobbinSize(BobbinActor::BobbinSize::Medium);
    }
    break;
    case STAGE_NAME::DIFFICULT:
        break;
    case STAGE_NAME::BOSS:
    {
#if 0
        Transform bobbinTr(DirectX::XMFLOAT3{ 6.0f,0.0f,18.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr);
        bobbin->SetBobbinSize(BobbinActor::BobbinSize::Big);

        Transform bobbinTr1(DirectX::XMFLOAT3{ 18.0f,0.0f,6.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr1);
        bobbin1->SetBobbinSize(BobbinActor::BobbinSize::Big);
#else
        Transform bobbinTr(DirectX::XMFLOAT3{ 12.0f,0.0f,12.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto bobbin = this->GetActorManager()->CreateAndRegisterActorWithTransform<BobbinActor>("BobbinActor", bobbinTr);
        bobbin->SetBobbinSize(BobbinActor::BobbinSize::BossBobbin);


#endif // 0
    }
    break;
    }

}

// ステージごとのプレイヤーの場所を設定する
void GameScene::SetupPlayerPos(STAGE_NAME stageId)
{
    switch (stageId)
    {
    case STAGE_NAME::TUTORIAL:
    case STAGE_NAME::FIRST:
        player->SetPosition({ 12.0f,0.0,7.0f });
    case STAGE_NAME::BOBBIN_FIRST:
        break;
    case STAGE_NAME::REFLECT_WALL:
        player->SetPosition({ 7.0f,0.0,9.0f });
        break;
    case STAGE_NAME::BOBBIN_SECOND:
    case STAGE_NAME::DIFFICULT:
    case STAGE_NAME::BOSS:
        break;
    }
}


// ステージごとのBGMを設定する
void GameScene::SetupBGM(STAGE_NAME stageId)
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    audioBgmComponent = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    switch (stageId)
    {
    case STAGE_NAME::TUTORIAL:
    case STAGE_NAME::FIRST:
    case STAGE_NAME::BOBBIN_FIRST:
    case STAGE_NAME::REFLECT_WALL:
    case STAGE_NAME::BOBBIN_SECOND:
    case STAGE_NAME::DIFFICULT:
        audioBgmComponent->SetSource(L"./Data/Sound/BGM1/game_bgm.wav");
        break;
    case STAGE_NAME::BOSS:
        audioBgmComponent->SetSource(L"./Data/Sound/BGM1/boss_bgm.wav");
        break;
    }
    audioBgmComponent->SetIsBgm(true);
    audioBgmComponent->SetLoop(true);
    audioBgmComponent->SetVolume(0.5f);
}

// STAGE_NAMEを変換する関数
STAGE_NAME GameScene::StringToStageName(const std::string& name)
{
    if (name == "TUTORIAL")
        return STAGE_NAME::TUTORIAL;

    if (name == "FIRST")
        return STAGE_NAME::FIRST;

    if (name == "BOBBIN_FIRST")
        return STAGE_NAME::BOBBIN_FIRST;

    if (name == "REFLECT_WALL")
        return STAGE_NAME::REFLECT_WALL;

    if (name == "BOBBIN_SECOND")
        return STAGE_NAME::BOBBIN_SECOND;

    if (name == "DIFFICULT")
        return STAGE_NAME::DIFFICULT;

    if (name == "BOSS")
        return STAGE_NAME::BOSS;

    return STAGE_NAME::FIRST;
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


    ImGui::Begin(U8("調整"));
    ImGui::SliderFloat4("morphWeight", &gameSceneCBuffer->data.morphWeights.x, 0.0f, 1.0f);
    if (ImGui::TreeNode(U8("ゲームオーバー演出")))
    {
        ImGui::DragFloat(U8("半径"), &gameOverRadius, 0.1f, -0.1f, 2.0f);
        ImGui::DragFloat(U8("円の中心の調整値Y"), &gameOverTargetPosY, 0.5f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(U8("敵")))
    {
        ImGui::DragFloat("Distance Dash", &enemyTuning.knockbackDistanceDash, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat("Distance Normal", &enemyTuning.knockbackDistanceNormal, 0.1f, 0.0f, 20.0f);

        ImGui::DragFloat("Height Dash", &enemyTuning.knockbackHeightDash, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat("Height Normal", &enemyTuning.knockbackHeightNormal, 0.1f, 0.0f, 20.0f);

        ImGui::DragFloat("Duration Dash", &enemyTuning.knockbackDurationDash, 0.01f, 0.1f, 2.0f);
        ImGui::DragFloat("Duration Normal", &enemyTuning.knockbackDurationNormal, 0.01f, 0.1f, 2.0f);

        ImGui::Separator();

        ImGui::DragFloat("Flash Duration", &enemyTuning.flashDuration, 0.01f, 0.05f, 2.0f);
        ImGui::DragFloat("Flash Sharpness", &enemyTuning.flashSharpness, 0.1f, 1.0f, 20.0f);
        ImGui::DragFloat("Emissive Power", &enemyTuning.emissivePower, 0.5f, 0.0f, 50.0f);

        ImGui::TreePop();
    }
    if (ImGui::TreeNode(U8("コイン")))
    {
        ImGui::DragFloat(U8("コインの上昇時間"), &normalCoin.duration, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat(U8("コインの上昇距離"), &normalCoin.height, 0.1f, 0.0f, 20.0f);

        ImGui::DragFloat(U8("コインのトレイルのスポーンの間隔"), &normalCoin.trailSpawnInterval, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat(U8("コインのトレイルのサイズ"), &normalCoin.trailSize, 2.f, 1.0f, 30.0f);

        ImGui::DragFloat(U8("コインのバーストのサイズ"), &normalCoin.burstSize, 2.f, 1.0f, 100.0f);
        ImGui::DragInt(U8("コインのバーストの個数"), &normalCoin.burstCount, 1, 1, 15);
        ImGui::DragFloat(U8("コインのバーストのスピード"), &normalCoin.burstShrinkSpeed, 2.f, 100.0f, 600.0f);

        ImGui::TreePop();
    }
    if (ImGui::TreeNode(U8("ボーナスコイン")))
    {
        ImGui::DragFloat(U8("コインの上昇時間"), &bonusCoin.duration, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat(U8("コインの上昇距離"), &bonusCoin.height, 0.1f, 0.0f, 20.0f);

        ImGui::DragFloat(U8("コインのトレイルのスポーンの間隔"), &bonusCoin.trailSpawnInterval, 0.1f, 0.0f, 20.0f);
        ImGui::DragFloat(U8("コインのトレイルのサイズ"), &bonusCoin.trailSize, 2.f, 1.0f, 100.0f);

        ImGui::DragFloat(U8("コインのバーストのサイズ"), &bonusCoin.burstSize, 2.f, 1.0f, 200.0f);
        ImGui::DragInt(U8("コインのバーストの個数"), &bonusCoin.burstCount, 1, 1, 50);
        ImGui::DragFloat(U8("コインのバーストのスピード"), &bonusCoin.burstShrinkSpeed, 2.f, 100.0f, 1000.0f);

        ImGui::TreePop();
    }

    ImGui::End();

#endif

}

// 仮の敵を生成する関数
void GameScene::SpawnEnemy(
    const XMFLOAT3& pos,
    YarnEnemyType type, bool isBig,
    float speed, const XMFLOAT3& dir)
{
    DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };

    if (isBig)
    {
        scale = { 1.0f, 1.0f, 1.0f };
    }
    else
    {
        scale = { 1.1f, 1.1f, 1.1f };
    }

    Transform tr(pos, { 0,180,0 }, scale);
    auto enemy = GetActorManager()->CreateAndRegisterActorWithTransform<EnemyBase>("enemy", tr);
    enemy->SetMoveDirection(dir);

    auto size = isBig ? EnemyBase::Big : EnemyBase::Small;
    enemy->SetEnemySize(size);
    enemy->SetEnemyType(type);

    switch (type)
    {
    case YarnEnemyType::Static:
        enemy->SetBehavior(std::make_unique<StaticBehavior>());
        break;

    case YarnEnemyType::MoveHorizontal:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        enemy->SetMoveDirection({ 1,0,0 });
        break;

    case YarnEnemyType::MoveVertical:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        enemy->SetMoveDirection({ 0,0,1 });
        break;

    case YarnEnemyType::MoveLinear:
        enemy->SetBehavior(std::make_unique<LinearBehavior>());
        break;

    case YarnEnemyType::WaveHorizontal:
        enemy->SetBehavior(std::make_unique<WaveHorizontalBehavior>());
        enemy->SetMoveDirection({ 1,0,0 });
        break;

    case YarnEnemyType::WaveVertical:
        enemy->SetBehavior(std::make_unique<WaveVerticalBehavior>());
        enemy->SetMoveDirection({ 0,0,1 });
        break;

    case YarnEnemyType::WaveMoveBehavior:
        enemy->SetBehavior(std::make_unique<WaveMoveBehavior>());
        break;

    case YarnEnemyType::ChasePlayer:
        enemy->SetBehavior(std::make_unique<ChaseBehavior>());
        break;
    case YarnEnemyType::RescueEnemy:
        enemy->SetBehavior(std::make_unique<RescueBehavior>());
        break;
    case YarnEnemyType::LongRangeAttack:
        enemy->SetAttack(std::make_unique<NeedleAttack>());
        break;
    }
    enemy->SetSpeed(speed);
    enemy->SetUpVisual();
}

