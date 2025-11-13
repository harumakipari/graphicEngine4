#include "SceneBase.h"

#include "ImGuizmo.h"

#include "Engine/Input/InputSystem.h"
#include "Graphics/PostProcess/BloomEffect.h"
#include "Graphics/PostProcess/FogEffect.h"
#include "Graphics/PostProcess/SSAOEffect.h"
#include "Graphics/PostProcess/SSREffect.h"


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

    fullscreenQuad = std::make_unique<FullScreenQuad>(device);

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

    return true;
}

void SceneBase::Update(float deltaTime)
{
    lightManager->Update(deltaTime);

    sceneCBuffer->data.elapsedTime += deltaTime;
    sceneCBuffer->data.deltaTime = deltaTime;

#ifdef _DEBUG
    if (InputSystem::GetInputState("F8", InputStateMask::Trigger))
    {
        CameraManager::ToggleCamera();
    }
#endif // !_DEBUG
}


bool SceneBase::OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height)
{
    framebufferDimensions.cx = static_cast<LONG>(width);
    framebufferDimensions.cy = static_cast<LONG>(height);

    cascadedShadowMaps = std::make_unique<decltype(cascadedShadowMaps)::element_type>(device, 1024 * 4, 1024 * 4);

    multipleRenderTargets = std::make_unique<decltype(multipleRenderTargets)::element_type>(device, framebufferDimensions.cx, framebufferDimensions.cy, 3);

    postEffectManager->Initialize(device, framebufferDimensions.cx, framebufferDimensions.cy);
    sceneEffectManager->Initialize(device, framebufferDimensions.cx, framebufferDimensions.cy);
    return true;
}

void SceneBase::UpdateConstantBuffer(ID3D11DeviceContext* immediateContext)
{
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

    sceneCBuffer->Activate(immediateContext, 1); 
    shaderCBuffer->Activate(immediateContext, 9);
    lightManager->Apply(immediateContext, 11);
}

void SceneBase::DrawGui()
{
#ifdef USE_IMGUI
    SetupImGuiStyle();

    DrawOutliner();
    DrawInspector();
    DrawShortcutInfo();
#endif
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

    ImGui::Text(" ShortcutInfo:");
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

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 300.0f,
        viewport->WorkPos.y + viewport->WorkSize.y - 100.0f));
    ImGui::SetNextWindowBgAlpha(0.3f); // 半透明

}


void SceneBase::DrawOutliner()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float left_panel_width = 300.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(left_panel_width, viewport->WorkSize.y));
    ImGui::Begin("Actor Outliner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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
    }
}


void SceneBase::DrawInspector()
{
    // 右側
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float right_panel_width = 400.0f;
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - right_panel_width, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(right_panel_width, viewport->WorkSize.y));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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
    ImGui::Checkbox("Enable Fog", &enableFog);
    ImGui::Checkbox("Enable CSM", &enableCascadedShadowMaps);

    sceneEffectManager->DrawGui();
    postEffectManager->DrawGui();
}


void SceneBase::DrawGizmo()
{
    //if (!selectedActor_) return;

    //ImGuizmo::BeginFrame();

    //ImGuizmo::SetOrthographic(false);
    //ImGuizmo::SetDrawlist();

    //ImGuiViewport* vp = ImGui::GetMainViewport();
    //ImGuizmo::SetRect(vp->WorkPos.x, vp->WorkPos.y, vp->WorkSize.x, vp->WorkSize.y);

    //auto& transform = selectedActor_->GetRootComponent()->GetTransform();
    //DirectX::XMMATRIX world = transform.GetWorldMatrix();
    //DirectX::XMMATRIX view = CameraManager::GetCurrentCamera()->GetViewMatrix();
    //DirectX::XMMATRIX proj = CameraManager::GetCurrentCamera()->GetProjMatrix();

    //ImGuizmo::Manipulate(
    //    (float*)&view, (float*)&proj,
    //    ImGuizmo::TRANSLATE, // ROTATE or SCALEもOK
    //    ImGuizmo::WORLD,
    //    (float*)&world
    //);

    //if (ImGuizmo::IsUsing()) {
    //    transform.SetFromMatrix(world); // ←ワールド行列から位置・回転・スケール再計算
    //}
}
