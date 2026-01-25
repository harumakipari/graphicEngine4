#include "pch.h"
#include "TutorialScene.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

#include <magic_enum.hpp>

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Input/InputSystem.h"
#include "Core/ActorManager.h"
#include "Engine/Utility/Time.h"

#include "Game/Actors/Stage/Cloth.h"
#include "Game/OdenGame/OdenNextViewActor.h"
#include "Game/OdenGame/OdenUIScoreViewActor.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"


#include "Physics/Physics.h"
#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Game/OdenGame/OdenActors/OdenStoreActor.h"
#include "Game/OdenGame/OdenActors/OdenTrashActor.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"
#include "Game/OdenGame/OdenActors/OdenDetailIngredientsActors.h"
#include "Game/OdenGame/BeatClockActor.h"
#include "Game/OdenGame/OdenGameSession.h"
#include "Game/OdenGame/OdenUITimerActor.h"


#include "Physics/CollisionSystem.h"
#include "UI/UIManager.h"
#include "UI/Game/Pause.h"
#include "Game/OdenGame/OdenManagers/OdenSlotManager.h"
#include "Game/OdenGame/OdenManagers/OdenOrderManager.h"
#include "Game/OdenGame/OdenManagers/OdenGameManager.h"
#include "Game/OdenGame/OdenData/OdenGameParameter.h"
#include "Game/OdenGame/OdenTutorial/TutorialActor.h"
#include "Game/OdenGame/OdenTutorial/TutorialManager.h"


//#ifdef _DEBUG
//#endif // _DEBUG

bool TutorialScene::Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props)
{
    SceneBase::Initialize(device, width, height, props);

    Physics::Instance().Initialize();

    // 具材のそれぞれの面のデータをロード
    OdenGameParameter::LoadOdenFaceShapeTableFromCSV("./Data/Game/OdenGameData.csv");

    // オーダーのデータをロード
    OdenGameParameter::LoadOrderDBFromCSV("./Data/Game/OdenOrderData.csv");

#if 0
    // 確認
    for (auto& [name, table] : OdenGameParameter::odenTypeShapes)
    {
        Logger::Log("Ingredient: " + name);
        for (auto& [face, data] : table.faceShapes)
        {
            auto faceName = std::string(magic_enum::enum_name(face));
            auto categoryName = std::string(magic_enum::enum_name(data.category));

            std::string logStr =
                "Face: " + faceName + "  Category:" + categoryName +
                " Roundness: " + std::to_string(data.property.roundness) +
                " Aspect: " + std::to_string(data.property.aspectRatio) +
                " Hole: " + std::to_string(data.property.holeNess);

            Logger::Log(logStr.c_str());
        }

        Logger::Log("--- OrderDB ---");
        Logger::Log("ShapeOrders: " + name);
        for (auto& entry : OrderDB.shapeOrders)
        {
            auto categoryName = std::string(magic_enum::enum_name(entry.data.requiredCategory));

            std::string logStr =
                "UI: " + entry.uiName + "  Category:" + categoryName;
            Logger::Log(logStr.c_str());
        }

        Logger::Log("IngredientOrders:");
        for (auto& entry : OrderDB.ingredientOrders)
        {
            auto ingredientName = std::string(magic_enum::enum_name(entry.data.requiredIngredient));
            std::string logStr =
                "UI: " + entry.uiName + "  Category:" + ingredientName;
            Logger::Log(logStr.c_str());
        }
    }
#endif // 0

    //アクターをセット
    SetUpActors();

    // 暖簾のモデルを作成
    // clothSimulate = std::make_unique<ClothSimulate>(device, "./Data/Models/Oden_Store/cloth1.gltf");

    // おでんの汁の定数バッファを作成
    odenSoupCBuffer = std::make_unique<ConstantBuffer<OdenSoupConstantBuffer>>(Graphics::GetDevice());

    // ここで布を描画する
    RegisterRenderHook(RenderPass::Opaque, [&](ID3D11DeviceContext* immediateContext)
        {
            if (const auto cloth = GetActorManager()->GetActorByName("cloth"))
            {
                //   clothSimulate->Render(immediateContext, cloth->GetWorldTransform());
            }
        });

    // 水のノーマルテクスチャを追加
    D3D11_TEXTURE2D_DESC texture2dDesc;
    HRESULT hr = LoadTextureFromFile(device, L"./Data/ShaderTextures/waterNormal.png", waterNormalTexture.GetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    return true;
}

void TutorialScene::Start()
{
    auto audioActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("Audio");
    auto audioBgmComponent = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioBgmComponent->SetSource(L"./Data/Sound/BGM/game.wav");
    audioBgmComponent->SetLoop(true);
    audioBgmComponent->SetVolume(0.8f);
    audioBgmComponent->Play();

    auto audioPotBgmComponent = audioActor->AddComponent<CoreAudioSourceComponent>("audioSource");
    audioPotBgmComponent->SetSource(L"./Data/Sound/BGM/pot_bgm.wav");
    audioPotBgmComponent->SetLoop(true);
    audioPotBgmComponent->SetVolume(3.0f);
    audioPotBgmComponent->Play();

    // 難易度設定を取得
    const auto& sceneTransition = SceneTransitionManager::Instance();
    const auto& params = sceneTransition.GetParams();

    if (params.contains("difficulty"))
    {
        difficulty = static_cast<GameDifficulty>(
            std::stoi(params.at("difficulty"))
            );
        OdenGameSession::SetDifficulty(difficulty);
    }

    difficulty = OdenGameSession::GetDifficulty();

    Logger::Log("Oden Game Difficulty: " + std::to_string(static_cast<uint8_t>(difficulty)));
#if 1

    // スロットマネージャー作成 
    slotManager = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotManager>("slotManager");
    //slotManager->StartGame();
    // チュートリアルでは回転を止める
    slotManager->SetRotationEnabled(false);
    // チュートリアルでは補充しない
    slotManager->SetSupplyEnabled(false);

    // 下段
    CreateSlotRow(GetActorManager(),"bottom",
        { "Chikuwa", "Daikon", "Egg", "Daikon" },0.0f,
        ERotationType::Horizontal);

    // 上段
    CreateSlotRow(GetActorManager(),"top",
        { "Daikon", "Konnyaku", "Kobumusubi", "Goboten" },4.0f,
        ERotationType::Vertical);

    // 右上に表示する次来る食材を表示する
    auto odenNextViewActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenNextViewActor>("odenNextViewActor");

    // ビートを設定する関数
    auto beatClockActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<BeatClockActor>("beatClockActor");
    slotManager->SetBeatActor(beatClockActor);
#else

#endif // 0
    // ステージアクターを生成
    Transform stageTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto stage = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenStoreActor>("stage", stageTr);
    slotManager->RegisterBeatReactive(stage);  // ビートするものとして設定


#if 0
    // デバック時に使用
    // お題を生成
    Transform odenBubbleTr(DirectX::XMFLOAT3{ 2.0f,3.0f,9.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto odenBubbleActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenBubbleActor>("odenBubble", odenBubbleTr);
    auto order = FindOrderByUIName("UI_Order_Daikon");
    odenBubbleActor->SetOrderAndMakeUi(order->data, order->uiName);

    // お題を生成
    Transform odenBubbleTr1(DirectX::XMFLOAT3{ 5.0f,3.0f,9.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto odenBubbleActor1 = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenBubbleActor>("odenBubble", odenBubbleTr1);
    order = FindOrderByUIName("UI_Order_CircleLike");
    odenBubbleActor1->SetOrderAndMakeUi(order->data, order->uiName);
#else

    // お題マネージャー作成
    // 内部で OdenBubbleActor をスロットマネージャーに設定している
    auto orderManager = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenOrderManager>("orderManager");
    // orderManager->SpawnSpecificOrderBubble(0, "UI_Order_Daikon");
    // orderManager->SpawnSpecificOrderBubble(1, "UI_Order_CircleLike");

#endif

    // ゴミ箱を生成
    Transform odenTrashTr(DirectX::XMFLOAT3{ -5.0f,1.0f,8.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto odenTrashActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<OdenTrashActor>("odenTrash", odenTrashTr);

    // ゲームマネージャーを生成
    auto gameManager = GetActorManager()->CreateAndRegisterActorWithTransform<OdenGameManager>("odenGameManager");
    gameManager->Reset();
    gameManager->EndGame(); // タイマーを進めないために

#if 1
    // スコアを表示するアクターを生成
    Transform uiScoreTr(DirectX::XMFLOAT3{ -7.1f,0.0f,-1.6f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto uiScoreViewActor = GetActorManager()->CreateAndRegisterActorWithTransform<OdenUIScoreViewActor>("OdenUIScoreViewActor", uiScoreTr);
    uiScoreViewActor->SetFontAndMakeTextComponent();

    // 時間を表示するアクターを生成
    auto uiTimerActor = GetActorManager()->CreateAndRegisterActorWithTransform<OdenUITimerActor>("OdenUITimerActor");

#endif // 0
    // チュートリアルアクターを生成
    auto tutorialActor = GetActorManager()->CreateAndRegisterActorWithTransform<TutorialActor>("OdenTutorialActor");

    SceneTransitionManager::Instance().SetOnOpeningFinished([this, tutorialActor]()
        {
            tutorialActor->StartTutorial();
        });

    // シーンが切り替わった時に
    SceneTransitionManager::Instance().NotifySceneChanged();

}

void TutorialScene::Update(float deltaTime)
{
    using namespace DirectX;
    SceneBase::Update(deltaTime);
    clothSimulate->Update(deltaTime);
    Physics::Instance().Update(Time::UnscaledDeltaTime());
    CollisionSystem::DetectAndResolveCollisions();
    CollisionSystem::ApplyPushAll();

}

void TutorialScene::SetUpActors()
{
    // メインカメラのターゲットアクターを生成
    Transform cameraTargetTr(DirectX::XMFLOAT3{ 5.4f,0.0f,4.3f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto mainCameraTarget = GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("MainCameraActorTarget", cameraTargetTr);

    // メインカメラアクターを生成
    auto mainCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<MainCamera>("mainCameraActor");
    auto mainCameraComponent = mainCameraActor->GetComponent<TPSCameraComponent>();
    mainCameraComponent->target = (mainCameraTarget->GetRootComponent());
    mainCameraComponent->pitch = DirectX::XMConvertToRadians(71.5f);
    mainCameraComponent->distance = 18.4f;
    SetActiveCamera(mainCameraActor);
    Logger::Log(U8("MainSceneのカメラ設定される。"));


#ifdef _DEBUG
    // デバックカメラアクターを生成
    auto debugCameraActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<DebugCamera>("debugCam");
    debugCameraActor->SetPosition({ 0.0f,10.0f,-20.0f });
    cameraManager->SetDebugCamera(debugCameraActor);
#endif // !_DEBUG

    // ポーズアクターを生成
    auto pauseActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Pause>("pauseActor");
    pauseActor->SetRetrySceneName("TutorialScene");

    // 暖簾を生成
    Transform clothTr(DirectX::XMFLOAT3{ 1.0f,9.0f,6.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 2.0f,2.0f,2.0f });
    auto clothActor = this->GetActorManager()->CreateAndRegisterActorWithTransform<Actor>("cloth", clothTr);


}

void TutorialScene::Render(ID3D11DeviceContext* immediateContext, float deltaTime)
{
    // 水のノーマルテクスチャを送る
    immediateContext->PSSetShaderResources(12, 1, waterNormalTexture.GetAddressOf());
    odenSoupCBuffer->data = odenSoupConstantBuffer;
    odenSoupCBuffer->Activate(immediateContext, 12);

    SceneBase::Render(immediateContext, deltaTime);
}


bool TutorialScene::Uninitialize(ID3D11Device* device)
{
    SceneBase::Uninitialize(device);
    Physics::Instance().Finalize();
    return true;
}

void TutorialScene::DrawGui()
{
#ifdef USE_IMGUI
    SceneBase::DrawGui();
    ImGui::Begin("OdenSoupBuffer");

    ImGui::ColorEdit4("shallowColor", &odenSoupConstantBuffer.shallowColor.x);
    ImGui::ColorEdit4("deepColor", &odenSoupConstantBuffer.deepColor.x);
    ImGui::SliderFloat("waterAlpha", &odenSoupConstantBuffer.waterAlpha, 0.0f, 1.0f);

    ImGui::DragFloat("normalScale", &odenSoupConstantBuffer.normalScale, 0.01f);
    ImGui::DragFloat("normalStrength", &odenSoupConstantBuffer.normalStrength, 0.01f);
    ImGui::DragFloat("normalSpeed", &odenSoupConstantBuffer.normalSpeed, 0.01f);
    ImGui::SliderFloat("specularSmoothness", &odenSoupConstantBuffer.specularSmoothness, 0.0f, 1.0f);

    ImGui::SliderFloat("specularHardness", &odenSoupConstantBuffer.specularHardness, 0.0f, 1.0f);
    ImGui::DragFloat("specularIntensity", &odenSoupConstantBuffer.specularIntensity, 0.01f);
    ImGui::ColorEdit3("specularColor", &odenSoupConstantBuffer.specularColor.x);
    ImGui::ColorEdit3("mainLightColor", &odenSoupConstantBuffer.mainLightColor.x);

    ImGui::DragFloat(U8("濁り"), &odenSoupConstantBuffer.turbidity, 0.01f);
    ImGui::DragFloat(U8("油膜"), &odenSoupConstantBuffer.oilStrength, 0.01f);

    ImGui::End();
#endif
}


void TutorialScene::CreateSlotRow(ActorManager* actorManager,
    const std::string& rowName,
    const std::vector<std::string>& ingredients,
    float startZ,
    ERotationType rotationType)
{
    for (int i = 0; i < ingredients.size(); ++i)
    {
        // Slot
        Transform slotTr(
            DirectX::XMFLOAT3{ i * 4.0f, 1.0f, startZ },
            DirectX::XMFLOAT4{ 0,0,0,1 },
            DirectX::XMFLOAT3{ 1,1,1 }
        );

        std::string slotName = rowName + "_slot_" + std::to_string(i);

        auto slot = actorManager
            ->CreateAndRegisterActorWithTransform<OdenSlotActor>(
                slotName,
                slotTr
            );

        slot->rotationType = rotationType;

        if (slotManager)
        {
            slotManager->RegisterSlot(slot);
        }

        // Ingredient（Slotと同じ位置）
        Transform ingredientTr = slotTr;

        std::string ingredientName = rowName + "_ingredient_" + std::to_string(i);

        auto ingredient = actorManager
            ->CreateAndRegisterActorWithTransform<OdenIngredientActor>(
                ingredientName,
                ingredientTr,
                ingredients[i]
            );

        // 相互セット
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);
    }
}