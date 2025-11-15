#include "LoadingScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Engine/Input/InputSystem.h"

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"



bool LoadingScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    OutputDebugStringA((std::string("Scene::Initialize this=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("_current_scene.get()=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("actorManager_ ptr=") + std::to_string(reinterpret_cast<uintptr_t>(this->GetActorManager())) + "\n").c_str());
    HRESULT hr;

    D3D11_BUFFER_DESC bufferDesc{};

    //splash = std::make_unique<Sprite>(device, L"./Data/Textures/Screens/TitleScene/994759-1.jpg");


    //hit_space_key = std::make_unique<Sprite>(device, L"./Data/Textures/Screens/LoadingScene/230x0w.png");

    bit_block_transfer = std::make_unique<FullScreenQuad>(device);
    //CreatePsFromCSO(device, "./Shader/DiscoTunnelPS.cso", pixel_shaders[0].ReleaseAndGetAddressOf());
    //CreatePsFromCSO(device, "./Shader/RoundedLoadingPS.cso", pixel_shaders[1].ReleaseAndGetAddressOf());
    //renderingState = std::make_unique<decltype(renderingState)::element_type>(device);

    bufferDesc.ByteWidth = sizeof(ShaderToyCB);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = device->CreateBuffer(&bufferDesc, nullptr, shaderToyConstantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));



    cbuffer = std::make_unique<ConstantBuffer<constants>>(device);


    //shaderToy
    shaderToyTransfer = std::make_unique<FullScreenQuad>(device);
    shaderToyFrameBuffer = std::make_unique<FrameBuffer>(device, 512, 512);

    // LoadSceneに持っていく用
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySky2.cso", pixel_shaders[0].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySkyPS.cso", pixel_shaders[1].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // ShaderToy
    //shaderToy = std::make_unique<ShaderToy>(device);

    type = std::stoi(props.at("type"));
    preload_scene = props.at("preload");
    _async_preload_scene(device, width, height, preload_scene);

    return true;
}

void LoadingScene::Start()
{
    SetUpActors();
}

void LoadingScene::SetUpActors()
{
    mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<TitleCamera>("mainLoadingCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();
    mainCameraActor->SetPosition({ -4.1f,1.9f,-4.3f });
    CameraManager::SetGameCamera(mainCameraActor.get());
    Transform enemyTr(DirectX::XMFLOAT3{ 14.8f,-6.0f,16.5f }, DirectX::XMFLOAT3{ 0.0f,35.0f,0.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    enemy = GetActorManager()->CreateAndRegisterActorWithTransform<EmptyEnemy>("Loadingenemy", enemyTr);
    enemy->PlayAnimation("Rotate", false);
}

void LoadingScene::Update(float deltaTime)
{
    SceneBase::Update(deltaTime);

    shaderToy.iTime += deltaTime;
    shaderToy.iResolution.x = Graphics::GetScreenWidth();
    shaderToy.iResolution.y = Graphics::GetScreenHeight();
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::None))
    {
        //shaderToy.iMouse.x = static_cast<float>(InputSystem::GetMousePositionX());
        //shaderToy.iMouse.y = static_cast<float>(InputSystem::GetMousePositionY());
    }

    //auto camera = std::dynamic_pointer_cast<TitleCamera>(ActorManager::GetActorByName("mainLoadingCameraActor"));
    //LoadingActorManager::Update(deltaTime);
    //if (InputSystem::GetInputState("Space", InputStateMask::Trigger))
    {
        if (_has_finished_preloading()/* && !enemy->GetAnimationController()->IsPlayAnimation()*/)
        {// 回転が三回したら
            //_transition(preload_scene, {});
        }
    }

#ifdef USE_IMGUI
    ImGui::Begin("Loading Scene");
    ImGui::DragFloat3("cameraTarget", &cameraTarget.x, 0.2f);
    ImGui::End();
#endif

}



bool LoadingScene::Uninitialize(ID3D11Device* device)
{
    //LoadingActorManager::ClearAll();
    //ActorManager::ClearAll();
    //enemy->SetPendingDestroy();
    //mainCameraActor->SetPendingDestroy();
    return true;
}

void LoadingScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
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

        //定数バッファをGPUに送信
        {
            immediateContext->UpdateSubresource(shaderToyConstantBuffer.Get(), 0, 0, &shaderToy, 0, 0);
            immediateContext->VSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf()); //register b7に送信
            immediateContext->PSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf());
        }


        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

        // Shadow Toy
        {
            //shaderToyFrameBuffer->Clear(immediateContext);// 512 * 512
            //shaderToyFrameBuffer->Activate(immediateContext);

            ID3D11ShaderResourceView* shaderResourceViews[]
            {
                //shaderToyFrameBuffer->shaderResourceViews[0].Get(), //color Map
                nullptr
            };
            //shaderToyTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, shaderToyPS.Get());
            shaderToyTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, pixel_shaders[type].Get());
            //shaderToyFrameBuffer->Deactivate(immediateContext);
        }


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


#if _DEBUG
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

        //定数バッファをGPUに送信
        {
            immediateContext->UpdateSubresource(shaderToyConstantBuffer.Get(), 0, 0, &shaderToy, 0, 0);
            immediateContext->VSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf()); //register b7に送信
            immediateContext->PSSetConstantBuffers(7, 1, shaderToyConstantBuffer.GetAddressOf());
        }


        RenderState::BindBlendState(immediateContext, BLEND_STATE::ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

        // Shadow Toy
        {
            //shaderToyFrameBuffer->Clear(immediateContext);// 512 * 512
            //shaderToyFrameBuffer->Activate(immediateContext);

            ID3D11ShaderResourceView* shaderResourceViews[]
            {
                nullptr
            };
            //shaderToyTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, shaderToyPS.Get());
            shaderToyTransfer->Blit(immediateContext, shaderResourceViews, 0, 1, pixel_shaders[type].Get());
            //shaderToyFrameBuffer->Deactivate(immediateContext);
        }


        RenderState::BindBlendState(immediateContext, BLEND_STATE::MULTIPLY_RENDER_TARGET_ALPHA);
        RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_ON);
        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_BACK);

        sceneRender.currentRenderPath = RenderPath::Deferred;
        sceneRender.RenderOpaque(immediateContext);
        sceneRender.RenderMask(immediateContext);
        //sceneRender.RenderBlend(immediateContext);

        RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::WIREFRAME_CULL_NONE);

        // デバック描画
#if _DEBUG
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
}


void LoadingScene::DrawGui()
{
    SceneBase::DrawGui();
}
