#include "pch.h"
#include "Framework.h"

#include <profiler.h>

#include "Graphics/Core/Shader.h"
#include <dxgi1_3.h>
#include <memory>
#include "Graphics/Resource/Texture.h"

#include "Engine/Scene/SceneRegistry.h"
#include "Graphics/Core/RenderState.h"

#include "Engine/Input/InputSystem.h"
#include "Graphics/Renderer/ShapeRenderer.h"
#include "Components/Audio/AudioSourceComponent.h"
#include "Engine/Audio/CoreAudio.h"
#include "Engine/Debug/DebugDrawManager.h"
#include "Engine/Debug/Logger.h"
#include "Engine/Effects/EffectEditor.h"
#include "Engine/Effects/EffectManager.h"


//コンストラクタ：ウィンドウハンドルを受け取って初期化
Framework::Framework(HWND hwnd, BOOL fullscreen) : hwnd(hwnd), fullscreenMode(fullscreen), windowed_style(static_cast<DWORD>(GetWindowLongPtrW(hwnd, GWL_STYLE)))
{
//#ifndef _DEBUG
    fullscreenMode = true;
//#endif
    Graphics::Initialize(hwnd, fullscreenMode);
    InputSystem::Initialize();
    RenderState::Initialize();
    Audio::Initialize();
    CoreAudio::Initialize();
}

bool Framework::Initialize()
{
    ////デバイス・デバイスコンテクスト・スワップチェーンの作成
    ID3D11Device* device = Graphics::GetDevice();
    if (!device) {
        assert("ModelComponent Error: device is null\n");
    }

    // ShapeRendererを初期化
    ShapeRenderer::Initialize(device);

    // SCENE_TRANSITION
    //Scene::_boot(device, "MainScene", SCREEN_WIDTH, SCREEN_HEIGHT, {});
    //Scene::_boot(device, "BootScene", SCREEN_WIDTH, SCREEN_HEIGHT, {});
    Scene::_boot(device, "SampleScene", SCREEN_WIDTH, SCREEN_HEIGHT, {});


    //パーティクルシステム
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleTexture;
    HRESULT hr = LoadTextureFromFile(device, L"./Data/Effect/Textures/particle.png", particleTexture.GetAddressOf(), NULL);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    particleSystem = std::make_unique<CoreComputeParticleSystem>(device, 30000, particleTexture);

    // エフェクトマネージャー初期化
    EffectManager::Initialize();

    //エフェクトエディタ初期化
    EffectEditor::Initialize();

    // ログ初期化
    Logger::Initialize();

    //プロファイラ初期化
    ProfileInitialize(&isPaused, Framework::SetPause/*, ImGuiControl::Profiler::DefaultMaxThreads*/);
    ProfileThreadName(0, "Main Thread");




    return true;
}

bool Framework::Update(float deltaTime/*Elapsed seconds from last frame*/)
{
    //デバイスコンテクストを取得
    ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();

    //オーディオ更新
    Audio::Update(deltaTime);
    CoreAudio::Update(deltaTime);

    // デバックコマンド更新
    DebugDrawManager::Tick(deltaTime);

    bool skipRendering;
    // SCENE_TRANSITION
    {
        ProfileScopedSection_2(0, "SceneUpdate", ImGuiControl::Profiler::Blue);
        skipRendering = Scene::_update(immediateContext, deltaTime);
    }

#ifdef USE_IMGUI
    ProfileNewFrame();
#endif

    //gameManager->UpdateAll(elapsed_time);
    //if (GetAsyncKeyState(VK_RETURN) & 1 && GetAsyncKeyState(VK_MENU) & 1)
    //{
    //    Graphics& graphics = Graphics::Instance();
    //    graphics.StylizeWindow(hwnd, !graphics.fullscreenMode);
    //}
    {
        ProfileScopedSection_2(0, "InputUpdate", ImGuiControl::Profiler::Green);
        //入力システム更新
        if (GetForegroundWindow() == Graphics::GetHwnd())
        {
            InputSystem::Update(Time::UnscaledDeltaTime());
        }
    }

    //パーティクルシステム更新
    {
        ProfileScopedSection_2(0, "ComputeParticleSystem::Update", ImGuiControl::Profiler::Blue);
        particleSystem->Update(Graphics::GetDeviceContext(), deltaTime);

        // エフェクトマネージャ更新
        EffectManager::Update(deltaTime);
    }


    return skipRendering;

}

void Framework::Render(float elapsed_time/*Elapsed seconds from last frame*/, bool skipRendering)
{
    HRESULT hr{ S_OK };

    //デバイスコンテクストを取得
    ID3D11DeviceContext* immediateContext = Graphics::GetDeviceContext();
    // サンプラーステートを設定
    RenderState::SetSamplerState(immediateContext);

    ID3D11RenderTargetView* nullRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    immediateContext->OMSetRenderTargets(_countof(nullRenderTargetViews), nullRenderTargetViews, 0);
    ID3D11ShaderResourceView* nullShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    immediateContext->VSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);
    immediateContext->PSSetShaderResources(0, _countof(nullShaderResourceViews), nullShaderResourceViews);

    //// 画面を初期化する（色を指定してレンダーターゲットをクリア）
    // 画面クリア
    Graphics::Clear(0.2f, 0.2f, 0.2f, 0.0f);

    // レンダーターゲット設定
    Graphics::SetRenderTargets();


    // SCENE_TRANSITION
    if (!skipRendering)
    {
        {
            ProfileScopedSection_2(0, "Render", ImGuiControl::Profiler::Red);
            Scene::_render(immediateContext, elapsed_time);
        }
        //gameManager->GenerateOutputAll();
    }

#ifdef USE_IMGUI
    //ImGui::Begin("ImGUI");
    {
        ImGui::PushFont(fontJP);
        ProfileScopedSection_2(0, "ImGui", ImGuiControl::Profiler::Yellow);
        Scene::_drawGUI();
        //Logger::DrawImGui();
        ImGui::PopFont();

    }

    /*ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
#if 0
    ImGui::Text("Video memory usage %d MB", video_memory_usage());
#endif
    ImGui::Text("ALT+ENTER to change window mode");

    ImGui::End();*/
#endif


#if 0
#ifdef USE_IMGUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

    //UINT sync_interval{ 0 };
    //swap_chain->Present(sync_interval, 0);
#endif


}

bool Framework::Uninitialize()
{
    //プロファイラ終了
    ProfileShutdown();

    ID3D11Device* device = Graphics::GetDevice();

    // エフェクトマネージャー終了
    EffectManager::ClearAll();

    CoreAudio::ClearAll();

    Audio::ClearAll();

    //gameManager->UninitAll();
    // SCENE_TRANSITION
    Scene::_uninitialize(device);
    return true;
}

Framework::~Framework()
{
    ReleaseAllTextures();
}
