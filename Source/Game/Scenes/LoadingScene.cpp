#include "pch.h"
#include "LoadingScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include <magic_enum.hpp>

#include "Engine/Input/InputSystem.h"

#include "Graphics/Core/Shader.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Resource/Texture.h"
#include "Graphics/Core/RenderState.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Game/OdenGame/OdenResultSkewerActor.h"
#include "Game/OdenGame/OdenLoadingIngredient.h"


bool LoadingScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);


    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<LoadingCamera>("mainLoadingCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<CameraComponent>();
    mainCameraActor->SetPosition({ -4.1f,1.9f,-4.3f });
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("ロードシーンのカメラ設定される。"));


    OutputDebugStringA((std::string("Scene::Initialize this=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("_current_scene.get()=") + std::to_string(reinterpret_cast<uintptr_t>(this)) + "\n").c_str());
    OutputDebugStringA((std::string("actorManager_ ptr=") + std::to_string(reinterpret_cast<uintptr_t>(this->GetActorManager())) + "\n").c_str());
    HRESULT hr;

    D3D11_BUFFER_DESC bufferDesc{};


    //shaderToy
    shaderToyCBuffer = std::make_unique<ConstantBuffer<ShaderToyCB>>(device);
    shaderToyTransfer = std::make_unique<FullScreenQuad>(device);
    shaderToyFrameBuffer = std::make_unique<FrameBuffer>(device, 512, 512);

    // LoadSceneに持っていく用
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySky2.cso", pixel_shaders[0].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    hr = CreatePsFromCSO(device, "./Shader/ShaderToySkyPS.cso", pixel_shaders[1].ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    // type = std::stoi(props.at("type"));
    preload_scene = props.at("preload");
    _async_preload_scene(device, width, height, preload_scene);

    //useDeferredRendering = false;
    loadingSprite = std::make_shared<Sprite>(device, L"./Data/Textures/UI/Oden_seane_change.png");

    return true;
}

void LoadingScene::Start()
{
    SetUpActors();

    //RegisterRenderHook(RenderPass::UI, [&](ID3D11DeviceContext* immediateContext)
    //    {
    //        //if (const auto e = GetActorManager()->GetActorByName("LoadingEnemy"))
    //        //{
    //            loadingSkewer->poleModel->RenderOpaque(immediateContext, loadingSkewer->GetWorldTransform());

    //            for (const auto ingredient:loadingSkewer->ingredients)
    //            {
    //                ingredient->LoadRenderIngredient(immediateContext);
    //            }
    //            //enemy->skeltalMeshComponent->RenderOpaque(immediateContext, e->GetWorldTransform());
    //        //}
    //    });

    float width = 1920.0f;
    float height = 1080.0f;

    sprite = std::make_shared<UISceneChangeComponent>("./Data/Textures/UI/Oden_seane_change.png", "sceneChange");
    sprite->SetWorldPosition({ width * 0.5f, height * 0.5f });
    sprite->SetPivot({ 0.5f,0.5f });
    sprite->SetSize({ width, height });
    sprite->zOrder = 1000;


    backImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Oden_seane_change.png", "backGround");
    backImage->SetSize({ 1920, 1080 });

#if 0
    GetUIManager()->Add(sprite);
#else
    RegisterRenderHook(RenderPass::Sky, [&](ID3D11DeviceContext* immediateContext)
        {
            backImage->Draw(immediateContext);
        });
#endif // 0
}

void LoadingScene::SetUpActors()
{
    // おでんの串を作る
    Transform skewerTr(
        XMFLOAT3{ 11.0f, .0f, 22.5f },
        XMFLOAT4{ 0,0,0,1 },
        XMFLOAT3{ 0.8f,0.8f,0.8f }
    );

    auto skewer = GetActorManager()
        ->CreateAndRegisterActorWithTransform<OdenResultSkewerActor>(
            "LoadingSkewer", skewerTr);

    auto types = CreateRandomSkewerIngredients();

    for (int i = 0; i < types.size(); ++i)
    {
        std::string name = std::string(magic_enum::enum_name(types[i]));

        auto ingredient = GetActorManager()
            ->CreateAndRegisterActorWithTransform<OdenLoadingIngredientActor>(
                "LoadingIngredient", Transform{}, name);

        skewer->AddIngredient(ingredient, i);
        ingredient->ingredientModel->SetIsVisible(true);
        
    }


    skewer->onRotationFinished = [this]()
        {
            if (_has_finished_preloading())
            {
                _transition(preload_scene, {});
            }
            else
            {
                canTransition = true;
            }
        };
    loadingSkewer = skewer; 
    loadingSkewer->StartRotateOneTurn();

    //Transform enemyTr(DirectX::XMFLOAT3{ 5.0f,-6.0f,16.5f }, DirectX::XMFLOAT3{ 0.0f,35.0f,0.0f }, DirectX::XMFLOAT3{ 2.0f,2.0f,2.0f });
    //enemy = GetActorManager()->CreateAndRegisterActorWithTransform<EmptyEnemy>("LoadingEnemy", enemyTr);
    //enemy->PlayAnimation("Idle", false);
}

void LoadingScene::Update(float deltaTime)
{
    SceneBase::Update(deltaTime);


    shaderToyConstant.iTime += deltaTime;
    shaderToyConstant.iResolution.x = Graphics::GetScreenWidth();
    shaderToyConstant.iResolution.y = Graphics::GetScreenHeight();

    if (canTransition && _has_finished_preloading())
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
    //loadingSprite->Render(immediateContext, 0, 0, 1920.0f, 1080.0f);
    // sprite->Draw(immediateContext);
}


void LoadingScene::DrawGui()
{
    SceneBase::DrawGui();
}

// ランダムのおでんの串を作る
std::vector<EOdenType> LoadingScene::CreateRandomSkewerIngredients()
{
    static std::array<EOdenType, 11> allTypes =
    {
        EOdenType::Daikon,
        EOdenType::Egg,
        EOdenType::Konnyaku,
        EOdenType::Hanpen,
        EOdenType::Chikuwa,
        EOdenType::Goboten,
        EOdenType::Cake,
        EOdenType::Shirataki,
        EOdenType::Kobumusubi,
        EOdenType::Tsukune,
        EOdenType::Donut,
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(allTypes.begin(), allTypes.end(), gen);

    return {
        allTypes[0],
        allTypes[1],
        allTypes[2],
    };
}