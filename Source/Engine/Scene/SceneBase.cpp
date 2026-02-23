#include "pch.h"
#include "SceneBase.h"
#include <profiler.h>
#ifdef USE_IMGUI
#include "ImGuizmo.h"
#endif
#include <DDSTextureLoader.h>

#include "Engine/Debug/DebugDrawManager.h"
#include "Engine/Debug/EditorGizmo.h"
#include "Engine/Debug/SceneEditor.h"
#include "Engine/Effects/EffectEditor.h"
#include "Engine/Effects/EffectManager.h"

#include "Engine/Input/InputSystem.h"
#include "Engine/Utility/Time.h"
#include "Game/Actors/Camera/Camera.h"
#include "Graphics/PostProcess/BloomEffect.h"
#include "Graphics/PostProcess/FogEffect.h"
#include "Graphics/PostProcess/SSAOEffect.h"
#include "Graphics/PostProcess/SSREffect.h"
#include "UI/FontManager.h"


bool SceneBase::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    sceneCBuffer = std::make_unique<ConstantBuffer<FrameConstants>>(device);
    shaderCBuffer = std::make_unique<ConstantBuffer<ShaderConstants>>(device);
    sceneCBuffer->data.elapsedTime = 0;//開始時に０にしておく

    // ライト
    {
        lightManager = std::make_unique<LightManager>();
        lightManager->Initialize(device);
        lightManager->SetDirectionalLight(lightDirection, lightColor);
    }

#if 1
    // ポストエフェクト
    {
        //if (!postEffectManager.get())
        {
            Logger::Log(U8("ポストエフェクトを作成しました！"));
            postEffectManager = std::make_unique<PostEffectManager>();
            postEffectManager->AddEffect(std::make_unique<BloomEffect>());
            postEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
        }
    }

    // シーンエフェクト
    {
        if (!sceneEffectManager.get())
        {
            Logger::Log(U8("シーンエフェクトを作成しました！"));
            sceneEffectManager = std::make_unique<SceneEffectManager>();
            sceneEffectManager->AddEffect(std::make_unique<FogEffect>());
            sceneEffectManager->AddEffect(std::make_unique<SSAOEffect>());
            sceneEffectManager->AddEffect(std::make_unique<SSREffect>());
            sceneEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
        }
    }

#endif // 0

    HRESULT hr = { S_OK };

    //スカイマップ
    skyMap = std::make_unique<decltype(skyMap)::element_type>(device, L"./Data/Environment/Sky/Night2/skybox.dds");

    fullscreenQuad = std::make_unique<FullScreenQuad>(device);

    // MULTIPLE_RENDER_TARGETS
    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, static_cast<uint32_t>(width), height, 3);

    frameBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);
    imGuiGizmoBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);

    // GBUFFER
    gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);
    hr = CreatePsFromCSO(device, "./Shader/DeferredLightingPS.cso", deferredPs.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //hr = CreatePsFromCSO(device, "./Shader/FinalPassPS.cso", finalPs.ReleaseAndGetAddressOf());
    hr = CreatePsFromCSO(device, "./Shader/FinalPS.cso", finalPs.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //CascadedShadowMaps
    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

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


    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    hr = DirectX::CreateDDSTextureFromFile(device, L"./Data/ShaderTextures/_noise_3d.dds", resource.ReleaseAndGetAddressOf(), noise3d.ReleaseAndGetAddressOf());
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

#if 0
    // シーンカラーテクスチャを送るのに使用する
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};

        // シーンカラー用
        Microsoft::WRL::ComPtr<ID3D11Resource> colorResource;
        frameBuffer->shaderResourceViews[0]->GetResource(colorResource.GetAddressOf());

        hr = colorResource->QueryInterface<ID3D11Texture2D>(sceneColorBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        sceneColorBuffer->GetDesc(&texture2dDesc);
        _ASSERT_EXPR(
            texture2dDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
            texture2dDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM ||
            texture2dDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT,
            "scene color must be a color format"
        );

        texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texture2dDesc.CPUAccessFlags = 0;
        texture2dDesc.MiscFlags = 0;

        hr = device->CreateTexture2D(&texture2dDesc, nullptr, sceneColorStencilBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        shaderResourceViewDesc.Format = texture2dDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(
            sceneColorStencilBuffer.Get(),
            &shaderResourceViewDesc,
            sceneColorSRV.GetAddressOf()
        );
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
#endif // 0


    dummyTexture = std::make_shared<UIImageComponent>("./Data/Textures/UI/Oden_seane_change.png", "backGround");
    dummyTexture->SetSize({ 1920, 1080 });

    return true;
}

void SceneBase::Update(float deltaTime)
{
    lightManager->Update(deltaTime);

    sceneCBuffer->data.elapsedTime += deltaTime;
    sceneCBuffer->data.deltaTime = deltaTime;

    uiManager->Update(deltaTime);

#ifdef _DEBUG
    if (InputSystem::GetInputState("F8", InputStateMask::Trigger))
    {
        cameraManager->ToggleCamera();
    }
#endif // !_DEBUG
}


bool SceneBase::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
{
    framebufferDimensions.cx = static_cast<LONG>(width);
    framebufferDimensions.cy = static_cast<LONG>(height);

    // cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);

    gBufferRenderTarget = std::make_unique<decltype(gBufferRenderTarget)::element_type>(device, static_cast<uint32_t>(width), height);

    //if (!postEffectManager.get())
    {
        Logger::Log(U8("ポストエフェクトを作成しました！"));
        postEffectManager = std::make_unique<PostEffectManager>();
        postEffectManager->AddEffect(std::make_unique<BloomEffect>());
        postEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
    }

    // シーンエフェクト
    {
        if (!sceneEffectManager.get())
        {
            Logger::Log(U8("シーンエフェクトを作成しました！"));
            sceneEffectManager = std::make_unique<SceneEffectManager>();
            sceneEffectManager->AddEffect(std::make_unique<FogEffect>());
            sceneEffectManager->AddEffect(std::make_unique<SSAOEffect>());
            sceneEffectManager->AddEffect(std::make_unique<SSREffect>());
            sceneEffectManager->Initialize(device, static_cast<uint32_t>(width), height);
        }
    }

    frameBuffer = std::make_unique<FrameBuffer>(device, static_cast<uint32_t>(width), height, false);

    return true;
}

void SceneBase::UpdateConstantBuffer(ID3D11DeviceContext* immediateContext)
{
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

    shaderCBuffer->data.enableSsao = enableSSAO;
    shaderCBuffer->data.enableBloom = enableBloom;
    shaderCBuffer->data.enableFog = enableFog;
    shaderCBuffer->data.enableCascadedShadowMaps = enableCascadedShadowMaps;
    shaderCBuffer->data.enableSsr = enableSSR;
    shaderCBuffer->data.enableBlur = enableBlur;

    sceneCBuffer->Activate(immediateContext, 1);
    shaderCBuffer->Activate(immediateContext, 9);

    // シーンからポイントライト集める
    lightManager->CollectPointLightsFromScene(*this);
    lightManager->Apply(immediateContext, 11);
}


void SceneBase::Render(ID3D11DeviceContext* immediateContext, float delta_time)
{
    if (auto camera = cameraManager->GetRenderCamera(this))
    {
        ViewConstants data = camera->GetViewConstants();
        sceneRender.UpdateViewConstants(immediateContext, data);
    }
    UpdateConstantBuffer(immediateContext);
#ifdef USE_IMGUI
    //imGuiGizmoBuffer->Clear(immediateContext);
    //imGuiGizmoBuffer->Activate(immediateContext);
#endif
    if (!useDeferredRendering)
    {// フォワードレンダリング
        ForwardRender(immediateContext);
    }
    else
    {
        DeferredRender(immediateContext);
    }
    Draw(immediateContext);
#ifdef USE_IMGUI
    //imGuiGizmoBuffer->Deactivate(immediateContext);
#endif
}


void SceneBase::ForwardRender(ID3D11DeviceContext* immediateContext)
{
    multipleRenderTargets->Clear(immediateContext);
    multipleRenderTargets->Activate(immediateContext);

    //auto camera = CameraManager::GetRenderCamera(this);
    auto camera = cameraManager->GetRenderCamera(this);
    if (!camera)
        return;

    ViewConstants data = camera->GetViewConstants();

    // スカイマップを描画
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    skyMap->Blit(immediateContext, data.viewProjection);
    ExecuteHooks(RenderPass::Sky, immediateContext);

    auto queues = sceneRender.BuildRenderQueues();


    // オブジェクトを描画
    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderOpaque(immediateContext, queues.deferredOpaque);
    sceneRender.RenderOpaque(immediateContext, queues.forwardOpaque);
    ExecuteHooks(RenderPass::Opaque, immediateContext);
    sceneRender.RenderMask(immediateContext, queues.deferredMask);
    sceneRender.RenderMask(immediateContext, queues.forwardMask);
    ExecuteHooks(RenderPass::Mask, immediateContext);
    sceneRender.RenderBlend(immediateContext, queues.deferredBlend);
    sceneRender.RenderBlend(immediateContext, queues.forwardBlend);
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

    // デバック描画
#if _DEBUG
    if (useDrawDebug)
    {
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);
        Physics::Instance().Render(data.view, data.projection, { lightManager->GetLightDirection().x,lightManager->GetLightDirection().y,lightManager->GetLightDirection().z });
        DebugDrawManager::Render(immediateContext);
        ExecuteHooks(RenderPass::Debug, immediateContext);
    }
#endif
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
    // PARTICLES
    {
        ProfileScopedSection_2(0, "Particles", ImGuiControl::Profiler::Green);

        //深度ステンシルステート設定
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF, 1);
        //ラスタライザ設定
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

        // パーティクル描画
        EffectManager::Render(immediateContext);

        ExecuteHooks(RenderPass::Particle, immediateContext);
    }


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
    // カスケードシャドウマップ生成
    cascadedShadowMaps->Clear(immediateContext);
    cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightManager->GetLightDirection(), criticalDepthValue, 3/*cbSlot*/);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    sceneRender.currentRenderPath = RenderPath::Shadow;
    sceneRender.CastShadowRender(immediateContext, queues.shadowCasters);
    cascadedShadowMaps->Deactive(immediateContext);

    // ファイナルパス
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

        sceneEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)], multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::NORMAL)],
            multipleRenderTargets->depthStencilShaderResourceView, multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::POSITION)], cascadedShadowMaps->depthMap().Get());
        postEffectManager->ApplyAll(immediateContext, multipleRenderTargets->renderTargetShaderResourceViews[0]);


        ID3D11ShaderResourceView* shader_resource_views[]
        {
            multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)],
            multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::POSITION)],
            multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::NORMAL)],
            multipleRenderTargets->depthStencilShaderResourceView,
            postEffectManager->GetOutput("BloomEffect"),
            sceneEffectManager->GetOutput("FogEffect"),
            sceneEffectManager->GetOutput("SSAOEffect"),
            sceneEffectManager->GetOutput("SSREffect"),
            cascadedShadowMaps->depthMap().Get(),
        };
        fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), finalPs.Get());
    }
}

void SceneBase::DeferredRender(ID3D11DeviceContext* immediateContext)
{
    //auto camera = CameraManager::GetRenderCamera(this);
    auto camera = cameraManager->GetRenderCamera(this);

    // ディファードレンダリング
    gBufferRenderTarget->Clear(immediateContext);
    gBufferRenderTarget->Acticate(immediateContext);

    auto queues = sceneRender.BuildRenderQueues();

    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    //RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_FRONT);
    sceneRender.currentRenderPath = RenderPath::Deferred;
    sceneRender.RenderOpaque(immediateContext, queues.deferredOpaque);
    ExecuteHooks(RenderPass::Opaque, immediateContext);

    sceneRender.RenderMask(immediateContext, queues.deferredMask);
    ExecuteHooks(RenderPass::Mask, immediateContext);

    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
    gBufferRenderTarget->Deactivate(immediateContext);

    DirectX::XMFLOAT4X4 cameraView;
    DirectX::XMFLOAT4X4 cameraProjection;

    if (camera)
    {
        ViewConstants data = camera->GetViewConstants();
        cameraView = data.view;
        cameraProjection = data.projection;
    }

    // 影を作る処理
    cascadedShadowMaps->Clear(immediateContext);
    cascadedShadowMaps->Activate(immediateContext, cameraView, cameraProjection, lightManager->GetLightDirection(), criticalDepthValue, 3/*cbSlot*/);
    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    sceneRender.currentRenderPath = RenderPath::Shadow;
    sceneRender.CastShadowRender(immediateContext, queues.shadowCasters);
    cascadedShadowMaps->Deactive(immediateContext);

    // ライティングのパス
    {
        //multipleRenderTargets->Clear(immediateContext);
        //multipleRenderTargets->Activate(immediateContext);

        frameBuffer->Clear(immediateContext);
        frameBuffer->Activate(immediateContext);

        // スカイマップを描画
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        if (camera)
        {
            ViewConstants data = camera->GetViewConstants();
            skyMap->Blit(immediateContext, data.viewProjection);
        }
        ExecuteHooks(RenderPass::Sky, immediateContext);

        //dummyTexture->Draw(immediateContext);

        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

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
        frameBuffer->Deactivate(immediateContext);
        //multipleRenderTargets->Deactivate(immediateContext);
    }

    //// 今回のゲームで追加
    //{
    //    immediateContext->CopyResource(sceneColorStencilBuffer.Get(), sceneColorBuffer.Get());
    //    immediateContext->PSSetShaderResources(25, 1, sceneColorSRV.GetAddressOf());
    //}

#if 1
    postEffectManager->ApplyAll(immediateContext, frameBuffer->shaderResourceViews[0].Get());
    sceneEffectManager->ApplyAll(immediateContext, frameBuffer->shaderResourceViews[0].Get(), gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],
        gBufferRenderTarget->depthStencilShaderResourceView, gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)], cascadedShadowMaps->depthMap().Get());
#endif

    // フォーワードの透明描画
    //multipleRenderTargets->Activate(immediateContext, gBufferRenderTarget->depthStencilView);

    //immediateContext->OMSetRenderTargets(1, gBufferRenderTarget->renderTargetViews[1], gBufferRenderTarget->depthStencilView);
    //immediateContext->OMSetRenderTargets(1, gBufferRenderTarget->renderTargetViews[4], gBufferRenderTarget->depthStencilView);


    frameBuffer->Activate(immediateContext, gBufferRenderTarget->depthStencilView);

#if 0
    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    sceneRender.currentRenderPath = RenderPath::Forward;
    sceneRender.RenderBlend(immediateContext); // ここで警告出る
#else

    RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_FRONT);
    sceneRender.currentRenderPath = RenderPath::Forward;
  //  sceneRender.RenderBlend(immediateContext, queues.deferredBlend); // ここで警告出る
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);

    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);
    sceneRender.currentRenderPath = RenderPath::Forward;
   // sceneRender.RenderBlend(immediateContext, queues.deferredBlend); // ここで警告出る
    ExecuteHooks(RenderPass::ForwardBlend, immediateContext);


#if 0
    // フォワードの描画
    {
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        sceneRender.RenderOpaque(immediateContext, queues.forwardOpaque);
        sceneRender.RenderMask(immediateContext, queues.forwardMask);
        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
        sceneRender.RenderBlend(immediateContext, queues.forwardBlend);
    }

#endif // 0

#endif // 0
    {
        //ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        //immediateContext->PSSetShaderResources(25, 1, nullSRV);
    }

#if 1
    // PARTICLES
    {
        ProfileScopedSection_2(0, "Particles", ImGuiControl::Profiler::Green);

        //深度ステンシルステート設定
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF, 1);
        //ラスタライザ設定
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

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
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);
        //Physics::Instance().Render(cameraView, cameraProjection, { lightDirection.x,lightDirection.y,lightDirection.z });
        DebugDrawManager::Render(immediateContext);
        ExecuteHooks(RenderPass::Debug, immediateContext);
    }
#endif
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

    frameBuffer->Deactivate(immediateContext);
    //multipleRenderTargets->Deactivate(immediateContext);


    immediateContext->PSSetShaderResources(20, 1, noise3d.GetAddressOf());


    // FINAL_PASS
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        //postEffectManager->ApplyAll(immediateContext, frameBuffer->shaderResourceViews[0].Get());
        //sceneEffectManager->ApplyAll(immediateContext, frameBuffer->shaderResourceViews[0].Get(), gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],
        //    gBufferRenderTarget->depthStencilShaderResourceView, gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)], cascadedShadowMaps->depthMap().Get());

        ID3D11ShaderResourceView* shader_resource_views[]
        {
            //  gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::COLOR)],   // colorMap
             frameBuffer->shaderResourceViews[0].Get(),//colorMap   こっちライティング済み
             //multipleRenderTargets->renderTargetShaderResourceViews[static_cast<int>(M_SRV_SLOT::COLOR)],//colorMap   こっちライティング済み
              gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::POSITION)],   // positionMap
              gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::NORMAL)],   // normalMap
              gBufferRenderTarget->depthStencilShaderResourceView,      //depthMap
              postEffectManager->GetOutput("BloomEffect"),
              sceneEffectManager->GetOutput("FogEffect"),
              sceneEffectManager->GetOutput("SSAOEffect"),
              sceneEffectManager->GetOutput("SSREffect"),
              gBufferRenderTarget->renderTargetShaderResourceViews[static_cast<int>(SRV_SLOT::EMISSIVE)],   // emissiveMap
              cascadedShadowMaps->depthMap().Get(),   //cascadedShadowMaps
        };
        immediateContext->PSSetShaderResources(8, 1, cascadedShadowMaps->depthMap().GetAddressOf());

        // メインフレームバッファとブルームエフェクトを組み合わせて描画
        fullscreenQuad->Blit(immediateContext, shader_resource_views, 0, _countof(shader_resource_views), finalPs.Get());

    }
}

void SceneBase::Draw(ID3D11DeviceContext* immediateContext)
{
    // UI描画
    {
        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);

        // 画像を表示
        uiManager->Draw(immediateContext);

        // フォントを表示
        uiManager->DrawFont(immediateContext);

        // シーン遷移用のUIを表示
        uiManager->DrawSceneChangeSprite(immediateContext);

        ExecuteHooks(RenderPass::UI, immediateContext);
    }
}

bool SceneBase::Uninitialize(ID3D11Device* device)
{
    uiManager->Clear();
    // エフェクトを全停止
    EffectManager::StopAll();

    return true;
}


void SceneBase::DrawGui()
{
#ifdef USE_IMGUI
    SetupImGuiStyle();
    //DrawDockSpace();
    DrawOutliner();
    //DrawGizmo();//
    Logger::DrawImGui();
    Time::DrawImGui();
    DrawInspector();
    SceneEditor::Draw();
    ProfileDrawUI();
    uiManager->DrawImGUi();
    EffectEditor::DrawGUI();
    DrawShortcutInfo();

    cascadedShadowMaps->DrawImGui();
#endif
}

void SceneBase::DrawDockSpace()
{
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpaceRoot", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(
        dockspace_id,
        ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_PassthruCentralNode
    );
    ImGui::End();
}

void SceneBase::DrawShortcutInfo()
{
    ImVec2 padding(10.0f, 10.0f);
    ImGuiViewport* viewport = ImGui::GetMainViewport();
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

    ImGui::Text(U8("ショートカットキー:"));
    ImGui::BulletText("Alt + Enter  : fullscreen");
    ImGui::BulletText("F8           : debugCamera");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#if 0
    ImGui::Text("Video memory usage %d MB", video_memory_usage());
#endif
    ImGui::End();
}


void SceneBase::SetupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.08f, 0.15f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.1f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.15f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.2f, 0.4f, 0.8f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.15f, 0.25f, 0.9f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.25f, 0.4f, 1.0f);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float screen_width = viewport->WorkSize.x;
    const float screen_height = viewport->WorkSize.y;

    const float left_panel_width = 300.0f;
    const float right_panel_width = 400.0f;

    //ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 300.0f,
    //    viewport->WorkPos.y + viewport->WorkSize.y - 100.0f));
    //ImGui::SetNextWindowBgAlpha(0.3f); // 半透明

}


void SceneBase::DrawOutliner()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float left_panel_width = 300.0f;
    //ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    //ImGui::SetNextWindowSize(ImVec2(left_panel_width, viewport->WorkSize.y));
    ImGui::Begin("Actor Outliner", nullptr/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);

    for (auto& actor : this->GetActorManager()->GetAllActors())
    {
        bool selected = (selectedActor_ == actor);
        if (ImGui::Selectable(actor->GetName().c_str(), selected))
        {
            selectedActor_ = actor;
        }
    }

    ImGui::End();
}

void SceneBase::DrawSceneSettingsTab()
{
    // -------------------------
// Light Settings
// -------------------------
    if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("useDeferredRendering", &useDeferredRendering);
        ImGui::Checkbox("useDrawDebug", &useDrawDebug);
        lightManager->DrawGUI();


    }

}


void SceneBase::DrawInspector()
{
    // 右側
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float right_panel_width = 400.0f;
    //ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - right_panel_width, viewport->WorkPos.y));
    //ImGui::SetNextWindowSize(ImVec2(right_panel_width, viewport->WorkSize.y));
    ImGui::Begin("Inspector", nullptr/*, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove*/);

    if (ImGui::BeginTabBar("InspectorTabs"))
    {
        if (ImGui::BeginTabItem("Actor"))
        {
            if (selectedActor_) selectedActor_->DrawImGuiInspector();
            else ImGui::Text("No actor selected.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("PostEffect"))
        {
            DrawPostEffectTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Scene"))
        {
            DrawSceneSettingsTab();
            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }
    ImGui::End();
}

void SceneBase::DrawPostEffectTab()
{
    ImGui::Checkbox("Enable SSAO", &enableSSAO);
    ImGui::Checkbox("Enable SSR", &enableSSR);
    ImGui::Checkbox("Enable Bloom", &enableBloom);
    ImGui::Checkbox("Enable Blur", &enableBlur);
    ImGui::Checkbox("Enable Fog", &enableFog);
    ImGui::Checkbox("Enable CSM", &enableCascadedShadowMaps);

    sceneEffectManager->DrawGui();
    postEffectManager->DrawGui();
    // -------------------------
    // CSM (シャドウ関連)
    // -------------------------
    if (ImGui::CollapsingHeader(U8("Cascaded Shadow Maps")))
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
    }

}


void SceneBase::DrawGizmo()
{
    ImGui::Begin("Viewport",
        nullptr,
        /* ImGuiWindowFlags_NoMove |*/
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    // =========================
    // ① 3D描画結果を表示
    // =========================
    constexpr float VIEW_W = 1280.0f;
    constexpr float VIEW_H = 720.0f;

    ImVec2 avail = ImGui::GetContentRegionAvail();

    // 中央寄せ
    ImVec2 offset(
        (avail.x - VIEW_W) * 0.5f,
        (avail.y - VIEW_H) * 0.5f
    );
    if (offset.x < 0) offset.x = 0;
    if (offset.y < 0) offset.y = 0;

    ImGui::SetCursorPos(ImGui::GetCursorPos() + offset);

    // === 3D描画 ===
    ImGui::Image(
        imGuiGizmoBuffer->shaderResourceViews[0].Get(),
        ImVec2(VIEW_W, VIEW_H)
    );

    // ★ Image の Rect を取得
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageSize = ImGui::GetItemRectSize();

    DirectX::XMMATRIX CameraView;
    DirectX::XMMATRIX CameraProjection;

    //if (auto camera = CameraManager::GetRenderCamera(this))
    if (auto camera = cameraManager->GetRenderCamera(this))
    {
        ViewConstants data = camera->GetViewConstants();
        CameraView = XMLoadFloat4x4(&data.view);
        CameraProjection = XMLoadFloat4x4(&data.projection);
    }

    if (InputSystem::GetInputState("W"))
        EditorGizmo::SetOperation(ImGuizmo::TRANSLATE);
    if (InputSystem::GetInputState("E"))
        EditorGizmo::SetOperation(ImGuizmo::ROTATE);
    if (InputSystem::GetInputState("R"))
        EditorGizmo::SetOperation(ImGuizmo::SCALE);

    if (selectedActor_)
    {
        auto root = selectedActor_->GetRootComponent();
        if (root)
        {
            EditorGizmo::Draw(
                root,
                CameraView,
                CameraProjection,
                imageMin,
                imageSize
            );
        }
    }

    ImGui::End();
#ifdef USE_IMGUI

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
#else
#endif

}
