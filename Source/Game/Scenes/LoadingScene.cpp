#include "pch.h"
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

    bit_block_transfer = std::make_unique<FullScreenQuad>(device);
    cbuffer = std::make_unique<ConstantBuffer<constants>>(device);

    //shaderToy
    shaderToyCBuffer = std::make_unique<ConstantBuffer<ShaderToyCB>>(device);
    shaderToyTransfer = std::make_unique<FullScreenQuad>(device);
    shaderToyFrameBuffer = std::make_unique<FrameBuffer>(device, 512, 512);

    // LoadSceneに持っていく用
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySky2.cso", pixel_shaders[0].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySkyPS.cso", pixel_shaders[1].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

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

    shaderToyConstant.iTime += deltaTime;
    shaderToyConstant.iResolution.x = Graphics::GetScreenWidth();
    shaderToyConstant.iResolution.y = Graphics::GetScreenHeight();
    if (_has_finished_preloading())
    {
        _transition(preload_scene, {});
    }
}



bool LoadingScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    return true;
}

void LoadingScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    //定数バッファをGPUに送信
    {
        shaderToyCBuffer->Activate(immediateContext, 7);
    }
    SceneBase::Render(immediateContext, deltaTime);
}


void LoadingScene::DrawGui()
{
    SceneBase::DrawGui();
}
