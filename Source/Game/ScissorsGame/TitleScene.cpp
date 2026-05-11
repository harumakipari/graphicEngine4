#include "pch.h"
#include "TitleScene.h"
#include <profiler.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#endif

#include "TitleBookActor.h"
#include "TitleCameraTargetActor.h"
#include "TitleStageActor.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"

#include "Physics/Physics.h"
#include "Game/ScissorsGame/ScissorsPlayer1.h"
#include "Game/ScissorsGame/ScissorsStage.h"
#include "Game/ScissorsGame/YarnEnemyActor.h"
#include "Graphics/PostProcess/BloomEffect.h"

#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"


bool TitleScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    lightDirection = { -0.65f, -0.38f, -0.0211f, 0.9f };   // 上の窓からの光
    lightColor = { 1.0f, 1.0f, 1.0f, 4.17f };
    {
        sceneCBuffer = std::make_unique<ConstantBuffer<FrameConstants>>(device);
        shaderCBuffer = std::make_unique<ConstantBuffer<SceneShaderConstants>>(device);
        sceneCBuffer->data.elapsedTime = 0;//開始時に０にしておく

        // ライト
        {
            lightManager = std::make_unique<LightManager>();
            lightManager->Initialize(device);
            lightManager->SetDirectionalLight(this,lightDirection, lightColor);
        }

        {
            {
                Logger::Log(U8("シーンエフェクトを作成しました！"));
                sceneEffectManager = std::make_unique<SceneEffectManager>();
                sceneEffectManager->AddEffect(std::make_unique<BloomEffect>());
                sceneEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
            }
        }

        HRESULT hr = { S_OK };

        //スカイマップ
        skyMap = std::make_unique<decltype(skyMap)::element_type>(device, L"./Data/Environment/Sky/Night2/skybox.dds");
        fullscreenQuad = std::make_unique<FullScreenQuad>(device);

        frameBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
        finalBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
        imGuiGizmoBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);

        // GBUFFER
        gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
        hr = CreatePsFromCSO(device, "./Shader/DeferredLightingPS.cso", deferredPs.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", finalPs.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //カスケードシャドウマップ
        cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024, 1024, 4);

        D3D11_TEXTURE2D_DESC texture2dDesc;
        //テクスチャをロード
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured_stage/lut_charlie.dds", environmentTextures[0].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured_stage/diffuse_iem.dds", environmentTextures[1].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured_stage/specular_pmrem.dds", environmentTextures[2].ReleaseAndGetAddressOf(), &texture2dDesc);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = LoadTextureFromFile(device, L"./Data/Environment/Sky/captured_stage/lut_sheen_e.dds", environmentTextures[3].ReleaseAndGetAddressOf(), &texture2dDesc);
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

    // シーンのライト設定などを設定する
    SceneSettings settings = {};
    settings.cascadedShadowMapConstants =
    {
        17.021f,
        0.136f,
        true,
        21.643f
    };
    settings.sceneShaderConstants =
    {
        0.75f,
        0.00011f,
        0.005f,
        0.0f,
        -0.028f,
        0.04f,
        0.018f,
        0.16f,
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
         { -0.65f, -0.38f, -0.0211f, 0.85f/* w:attenuation Rate */},
         { 1.0f, 0.8f, 1.0f, 4.17f/*w colorPower*/ },
         3.412f,
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

    return true;
}

void TitleScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioComp = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioComp->SetSource(L"./Data/Sound/BGM1/title_bgm.wav");
    audioComp->SetLoop(true);
    audioComp->Play();
    audioComp->SetVolume(0.8f);

#if 1

    // FIRST の作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/number/1.png", "1-1");
        button->SetWorldPosition({ 300, 50 });
        button->SetSize({ 400, 150 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"ボタン1-1Button Clicked!");
                //   Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","0"} });
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "GameScene"), {"stage","FIRST"} });
                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

    // BOBBIN_FIRST の作成
    {
        std::shared_ptr<UIButtonComponent> button = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/number/2.png", "1-2");
        button->SetWorldPosition({ 400, 50 });
        button->SetSize({ 400, 250 });
        uiManager->Add(button);

        button->onClick = []()
            {
                Logger::Log(u8"ボタン1-2Button Clicked!");
                //   Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","0"} });
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "GameScene"), {"stage","BOBBIN_FIRST"} });

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
                //Scene::_transition("LoadingScene", { std::make_pair("preload", "GameScene"),{"difficulty","1"} });
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","1"} });
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
                //Scene::_transition("LoadingScene", { std::make_pair("preload", "GameScene"),  {"difficulty","2"} });

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
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TutorialScene"), {"difficulty","1"} });

                CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav");
            };
    }

#endif // 1


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


    if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        const char* types[] = { "0", "1" };

        SceneTransitionManager::Instance().RequestTransition("GameScene");
    }
}

// 定数バッファの更新処理をシーンごとにカスタマイズできるようにするための仮想関数
void TitleScene::UpdateConstants(ID3D11DeviceContext* immediateContext, float deltaTime)
{

}

void TitleScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
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

    frameBuffer->Activate(immediateContext, gBufferRenderTarget->depthStencilView);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_FRONT);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend); // ここで警告出る
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_BACK);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend); // ここで警告出る
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

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

#ifdef USE_IMGUI
    imGuiGizmoBuffer->Deactivate(immediateContext);
#endif

}

// セレクトシーンへ
void TitleScene::StartToSelect()
{
    // メインカメラ
    mainCameraActor->Play(toSelectInterval);
    // メインカメラターゲットアクター
    cameraTargetActor->Play(toSelectInterval);
    // 本のアクター
    bookActor->Play(toSelectInterval);
}

// タイトルシーンへ
void TitleScene::StartToTitle()
{
    // メインカメラ
    mainCameraActor->PlayReverse(toTitleInterval);
    // メインカメラターゲットアクター
    cameraTargetActor->PlayReverse(toTitleInterval);
    // 本のアクター
    bookActor->PlayReverse(toTitleInterval);
}


void TitleScene::SetUpActors()
{
    Transform mainCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<FixedCamera>("fixedCameraActor", mainCameraTr);
    mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleCamera>("fixedCameraActor", mainCameraTr);
    mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();

    Transform cameraTargetTr(DirectX::XMFLOAT3{ -0.297f,3.197f,2.936f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    //Transform cameraTargetTr(DirectX::XMFLOAT3{ 2.2f,1.984f,2.753f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    cameraTargetActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleCameraTargetActor>("cameraTargetActor", cameraTargetTr);
    cameraTargetActor->SetTitle(true);

    mainCameraActor->SetTarget(cameraTargetActor->GetRootComponent());

    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->SetPitch(DirectX::XMConvertToRadians(-11.0f));
    mainCameraComponent->SetYaw(DirectX::XMConvertToRadians(231.5f));
    mainCameraComponent->SetFov(DirectX::XMConvertToRadians(24.0f));
    mainCameraComponent->distance = 8.945f;
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("タイトルシーンのカメラ設定される。"));

    Transform debugCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam", debugCameraTr);
    cameraManager->SetDebugCamera(debugCameraActor);

    Transform cinemaCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto cinemaCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<CinemaCamera>("cinemaCam", cinemaCameraTr);
    cameraManager->SetCinematicCamera(cinemaCameraActor);

    Transform movieCameraTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto movieCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MovieCamera>("movieCam", movieCameraTr);
    cameraManager->SetMovieCamera(movieCameraActor);

    Transform stageTr(DirectX::XMFLOAT3{ -0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stageActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleStageActor>("titleStageActor", stageTr);

    Transform bookTr(DirectX::XMFLOAT3{ 0.0f,0.5f,0.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    bookActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleBookActor>("BookActor", bookTr);
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

    ImGui::Begin(U8("調整"));

    ImGui::DragFloat(U8("選択シーンへの間隔時間"), &toSelectInterval,0.1f,0.0f,6.0f);
    if (ImGui::Button(U8("選択シーンへ")))
    {
        StartToSelect();
    }
    ImGui::DragFloat(U8("タイトルシーンへ間隔時間"), &toTitleInterval, 0.1f, 0.0f, 6.0f);
    if (ImGui::Button(U8("タイトルシーンへ")))
    {
        StartToTitle();
    }


    ImGui::End();
#endif

}

