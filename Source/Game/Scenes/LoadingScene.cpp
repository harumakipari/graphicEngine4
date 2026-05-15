#include "pch.h"
#include "LoadingScene.h"

#include "Engine/Framework/Framework.h"
#include "Game/ScissorsGame/StageData.h"

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

    preload_scene = props.at("preload");
    _async_preload_scene(device, width, height, preload_scene);

    //loadingSprite = std::make_shared<Sprite>(device, L"./Data/Textures/UI/Oden_seane_change.png");


    // チップスデータを設定する
    SetTipsData();


    loadingTime = 2.5f;   // ロードにかかる時間

    auto& param = SceneTransitionManager::Instance().GetParams();
    if (param.contains("fade"))
    {// ゲームオーバーだったら
        std::string name = param.at("fade");
        if (name == "0")
        {
            loadingSprite = std::make_shared<Sprite>(device, L"./Data/Textures/ScissorsUI/black.png");
            //gameOverSprite =std::make_shared<Sprite>(device, L"./Data/Textures/ScissorsUI/gameOver.png");
            loadingTime = 0.0f;
        }
    }

    return true;
}

void LoadingScene::Start()
{
    SetUpActors();

    RegisterRenderHook(RenderPass::UI, [&](ID3D11DeviceContext* immediateContext)
        {
            //if (const auto e = GetActorManager()->GetActorByName("LoadingEnemy"))
            //{
            //loadingSkewer->poleModel->RenderOpaque(immediateContext, loadingSkewer->GetWorldTransform());

            //for (const auto ingredient : loadingSkewer->ingredients)
            //{
            //    ingredient->LoadRenderIngredient(immediateContext);
            //}
            //enemy->skeltalMeshComponent->RenderOpaque(immediateContext, e->GetWorldTransform());
        //}
        });

    float width = 1920.0f;
    float height = 1080.0f;

    sprite = std::make_shared<UISceneChangeComponent>("./Data/Textures/UI/Oden_seane_change.png", "sceneChange");
    sprite->SetWorldPosition({ width * 0.5f, height * 0.5f });
    sprite->SetPivot({ 0.5f,0.5f });
    sprite->SetSize({ width, height });
    sprite->zOrder = 1000;


    backImage = std::make_shared<UIImageComponent>("./Data/Textures/UI/Oden_seane_change.png", "backGround");
    backImage->SetSize({ 1920, 1080 });


    std::shared_ptr<Sprite> chipSprite = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png");

    chipsImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tips/operate_1.png", "chipsImage");
    chipsImage->SetSize({ 440, 132 });
    chipsImage->SetWorldPosition({ 20, 880 });
    chipsImage->SetTexture(chipSprite);

    ApplyTipsTextures();

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
}

void LoadingScene::Update(float deltaTime)
{
    SceneBase::Update(deltaTime);

    loadingTime -= deltaTime;


    if (_has_finished_preloading() && loadingTime <= 0.0f)
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
        //shaderToyCBuffer->Activate(immediateContext, 7);
    }
    //SceneBase::Render(immediateContext, deltaTime);
    backImage->Draw(immediateContext);
    chipsImage->Draw(immediateContext);
    if (loadingSprite)
    {
        loadingSprite->Render(immediateContext, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        //gameOverSprite->Render(immediateContext, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
}


void LoadingScene::DrawGui()
{
    SceneBase::DrawGui();
}

// チップスのカテゴリーを決定する
LoadingScene::TipsCategory LoadingScene::DecideTipsCategory(const std::string& fromScene, const std::string& toScene)
{
    if (fromScene == "StageSelect" && toScene == "GameScene")
    {// ステージ選択からゲームシーン
        return TipsCategory::StageHint;
    }

    if (fromScene == "GameScene" && toScene == "ResultScene")
    {// ゲームシーンからリザルトへ
        return TipsCategory::EnemyLore;
    }

    if (fromScene == "ResultScene" && toScene == "TitleScene")
    {// リザルトシーンからタイトルシーンへ
        return TipsCategory::Funny;
    }

    if (fromScene == "GameScene" && toScene == "GameScene")
    {// ゲームシーンからゲームシーン
        return TipsCategory::Gameplay;
    }

    return TipsCategory::WorldLore;
}

// チップスデータ登録
void LoadingScene::SetTipsData()
{
    tipsDatabase =
    {
        {
            TipsCategory::StageHint,
            "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png"
            }
        },
        {
            TipsCategory::EnemyLore,
            "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore.png"
            }
        }

    };
}

// チップステクスチャを適応する
void LoadingScene::ApplyTipsTextures()
{
    auto& param = SceneTransitionManager::Instance().GetParams();

    std::string fromScene;
    std::string stage;

    if (param.contains("fromScene"))
    {// どこのシーンからきて
        fromScene = param.at("fromScene");

        Logger::Log("fromScene" + fromScene);

    }

    if (param.contains("stage"))
    {// 何のステージを遊ぶかor遊んだか
        stage = param.at("stage");

        Logger::Log("stage" + stage);
    }

    // 次にどこのシーンに行くか
    Logger::Log("to NextScene" + preload_scene);

    // カテゴリーを選択する
    TipsCategory category = DecideTipsCategory(fromScene, preload_scene);

    // 候補を集める
    std::vector<std::wstring> candidates;

    for (const auto& tip : tipsDatabase)
    {
        // カテゴリー一致
        if (tip.category != category)
            continue;

        // ステージ一致
        if (!tip.stage.empty() && tip.stage != stage)
            continue;

        for (const auto& tex : tip.textures)
        {
            candidates.push_back(tex);
        }
    }

    // 候補なし
    if (candidates.empty())
    {
        Logger::Warning("Tips texture not found.");
        return;
    }

    // ランダム
    int index = MathHelper::RandomRange(0, static_cast<int>(candidates.size()));

    // テクスチャを生成
    auto sprite = std::make_shared<Sprite>(Graphics::GetDevice(), candidates[index].c_str());

    // UIImageへ設定
    chipsImage->SetTexture(sprite);
}