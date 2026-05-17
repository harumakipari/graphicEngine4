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
#include "Game/ScissorsGame/ScoreCalculator.h"


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

    chipsImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/Tips/player_lore.png", "chipsImage");
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
        return TipsCategory::StageStart;
    }

    if (fromScene == "StageSelect" && toScene == "TutorialScene")
    {// ステージ選択からゲームシーン
        return TipsCategory::StageStart;
    }

    if (fromScene == "GameScene" && toScene == "ResultScene")
    {// ゲームシーンからリザルトへ
        return TipsCategory::Result;
    }

    if (fromScene == "ResultScene" && toScene == "TitleScene")
    {// リザルトシーンからタイトルシーンへ
        return TipsCategory::ReturnTitle;
    }

    if (fromScene == "GameScene" && toScene == "GameScene")
    {// ゲームシーンからゲームシーン
        return TipsCategory::Retry;
    }

    if (fromScene == "GameScene" && toScene == "TitleScene")
    {// ゲームシーンからゲームシーン
        return TipsCategory::ReturnTitle;
    }

    if (fromScene == "GameScene" && toScene == "TutorialScene")
    {// ゲームシーンからゲームシーン
        return TipsCategory::Retry;
    }

    if (fromScene == "TutorialScene" && toScene == "TitleScene")
    {// ステージ選択からタイトルシーン
        return TipsCategory::ReturnTitle;
    }

    return TipsCategory::ReturnTitle;
}

// チップスデータ登録
void LoadingScene::SetTipsData()
{
    tipsDatabase =
    {
        {
            TipsCategory::StageStart,
            "TUTORIAL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/player_lore.png"
            }
        },
        {
            TipsCategory::StageStart,
            "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png"
            }
        },
        {
            TipsCategory::StageStart,
            "BOBBIN_FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_hint.png"
            }
        },
        {
            TipsCategory::StageStart,
            "REFLECT_WALL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/redirect_hint_1.png"
            },
            false,
            true,
        },
        {
            TipsCategory::StageStart,
            "REFLECT_WALL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/redirect_hint_1_control.png"
            },
            true,
            false,
        },

        {
            TipsCategory::StageStart,
            "DIFFICULT",
            {
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png"
            }
        },
        {
            TipsCategory::StageStart,
            "BOSS",
            {
                 L"./Data/Textures/ScissorsUI/Tips/boss_hint_1.png"
            }
        },
        {
            TipsCategory::Result,
            "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
            }
        },
        {
            TipsCategory::Result,
            "BOBBIN_FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
            }
        },
        {
            TipsCategory::Result,
            "REFLECT_WALL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
            }
        },
        {
            TipsCategory::Result,
            "DIFFICULT",
            {
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_2.png",
            }
        },
        {
            TipsCategory::Result,
            "BOSS",
            {
                 L"./Data/Textures/ScissorsUI/Tips/boss_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/boss_lore_2.png",
            }
        },
        {
            TipsCategory::Retry,
            "TUTORIAL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
            }
        },

        {
            TipsCategory::Retry,
            "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
            }
        },
        {
            TipsCategory::Retry,
            "BOBBIN_FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_retry.png",
            }
        },
        {
            TipsCategory::Retry,
            "REFLECT_WALL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/redirect_retry.png",
            }
        },
        {
            TipsCategory::Retry,
            "DIFFICULT",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
            }
        },
        {
            TipsCategory::Retry,
            "BOSS",
            {
                 L"./Data/Textures/ScissorsUI/Tips/boss_retry_1.png",
            }
        },
        {
            TipsCategory::ReturnTitle,
                "TUTORIAL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
            }
        },

        {
            TipsCategory::ReturnTitle,
                "FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
            }
        },
        {
            TipsCategory::ReturnTitle,
                "BOBBIN_FIRST",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_lore_1.png",
            }
        },
        {
            TipsCategory::ReturnTitle,
                "REFLECT_WALL",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/redirect_retry.png",
            }
        },
        {
            TipsCategory::ReturnTitle,
                "DIFFICULT",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/redirect_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_3.png",
            }
        },
        {
            TipsCategory::ReturnTitle,
                "BOSS",
            {
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/game_hint_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_hint_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/scissors_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_hint.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/bobbin_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/redirect_retry.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/needle_lore_3.png",
                 L"./Data/Textures/ScissorsUI/Tips/boss_lore_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/boss_lore_2.png",
                 L"./Data/Textures/ScissorsUI/Tips/boss_retry_1.png",
                 L"./Data/Textures/ScissorsUI/Tips/boss_hint_1.png",
            }
                }


    };
}

// チップステクスチャを適応する
void LoadingScene::ApplyTipsTextures()
{
    auto& param = SceneTransitionManager::Instance().GetParams();

    std::string fromScene;

    bool usingGamepad = InputSystem::IsGamepadConnected();

    if (param.contains("fromScene"))
    {// どこのシーンからきて
        fromScene = param.at("fromScene");

        Logger::Log("fromScene" + fromScene);

    }


    auto stats = ScoreSystem::GetResultStats();
    std::string stage;
    if (param.contains("stage"))
    {// 何のステージを遊ぶかor遊んだか
        stage = param.at("stage");

        Logger::Log("stage" + stage);
    }

    stage = std::string(magic_enum::enum_name(stats.stageName));


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

        // パッド専用
        if (tip.gamePadOnly &&
            !usingGamepad)
        {
            continue;
        }

        // キーボード専用
        if (tip.keyboardOnly &&
            usingGamepad)
        {
            continue;
        }

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

    int index = 0;
    // ランダム
    if (candidates.size() == 1)
    {
        index = 0;
    }
    else
    {
        index = MathHelper::RandomRange(0, (static_cast<int>(candidates.size()) - 1));
    }

    Logger::Log("usingGamepad = " + std::to_string(usingGamepad));
    Logger::Log("category = " + std::to_string((int)category));
    Logger::Log("stage = " + stage);
    Logger::Log("candidate size = " + std::to_string(candidates.size()));

    // テクスチャを生成
    auto sprite = std::make_shared<Sprite>(Graphics::GetDevice(), candidates[index].c_str());

    // UIImageへ設定
    chipsImage->SetTexture(sprite);
}