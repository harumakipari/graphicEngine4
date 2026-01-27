#include "pch.h"
#include "OdenTitleStageActor.h"

#include <magic_enum.hpp>

#include "Engine/Scene/Scene.h"
#include "Game/Scenes/TitleScene.h"
#include "Physics/CollisionFunction.h"

void OdenTitleStageActor::Initialize(const Transform& transform)
{
    std::string parentName = "RootComponent";

    storeModelComponent = AddComponent<StaticMeshComponent>("Oden_Store_Model", parentName);
    storeModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Yatai.gltf", false);
    storeModelComponent->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });
    storeModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    auto groundModelComponent = AddComponent<StaticMeshComponent>("Oden_Ground_Model", parentName);
    groundModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Ground.gltf", false);
    groundModelComponent->SetRelativeScaleDirect({ 1.5f,1.0f,-1.0f });
    groundModelComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    struct InitData
    {
        Difficulty diff;
        const char* modelPath;
        DirectX::XMFLOAT3 pos;
        const char* diffStr;
    };

    InitData datas[] =
    {
        {Difficulty::Tutorial, "./Data/Models/Oden_Title_Stage/Oden_Title_Select_Tutorial.gltf", {10.7f,12.4f,7.6f}, "0"},
        {Difficulty::Easy,     "./Data/Models/Oden_Title_Stage/Oden_Title_Select_Easy.gltf",     {4.1f,12.4f,7.6f}, "0"},
        {Difficulty::Normal,   "./Data/Models/Oden_Title_Stage/Oden_Title_Select_Normal.gltf",   {-2.5f,12.4f,7.6f}, "1"},
        {Difficulty::Hard,     "./Data/Models/Oden_Title_Stage/Oden_Title_Select_Hard.gltf",     {-9.1f,12.4f,7.6f}, "2"},
    };

    for (auto& d : datas)
    {
        std::string name = "select_model_" + std::string(magic_enum::enum_name(d.diff).data());
        std::string boxName = "select_box_" + std::string(magic_enum::enum_name(d.diff).data());

        auto model = AddComponent<SkeletalMeshComponent>(name, "Oden_Store_Model");
        model->SetModel(d.modelPath);
        model->SetRelativeLocationDirect(d.pos);

        auto box = AddComponent<BoxComponent>(boxName, name);
        box->SetBoxExtent({ 3.0f, 4.5f, 0.3f });
        box->SetLayer(CollisionLayer::WorldStatic);
        box->Initialize();

        difficultySelects.push_back({ d.diff, model, box });
    }

#if 0
    // 難易度選択のモデル
    selectModelComponent = AddComponent<SkeletalMeshComponent>("Oden_Select_Model", "Oden_Store_Model");
    selectModelComponent->SetModel("./Data/Models/Oden_Title_Stage/Oden_Title_Select_Easy.gltf");
    selectModelComponent->SetRelativeLocationDirect({ 10.7f,12.4f,7.6f });

    // 選択のモデルの当たり判定を追加
    selectModelBoxComponent = AddComponent<BoxComponent>("Oden_BoxComponent", "Oden_Select_Model");
    //DirectX::XMFLOAT3 size = selectModelComponent->GetModelSize();
    selectModelBoxComponent->SetBoxExtent({ 3.0f,4.5f,0.3f });
    selectModelBoxComponent->SetMass(40.0f);
    selectModelBoxComponent->SetLayer(CollisionLayer::WorldStatic);
    selectModelBoxComponent->Initialize();

#endif // 0
}

void OdenTitleStageActor::Update(float elapsedTime)
{
    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;


    auto scene = Scene::GetCurrentScene();
    auto titleScene = dynamic_cast<TitleScene*>(scene);
    if (titleScene)
    {
        if (titleScene->GetPhase() != TitleScene::TitlePhase::DifficultySelect)
        {// 選択画面じゃなかったら
            return;
        }
    }

    // 重なっていたら
    HitResultWithActor result;

    bool hit = CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic));

    for (auto& d : difficultySelects)
    {
        bool hitThis = hit && result.component == d.collider.get();

        d.model->SetRelativeScaleDirect(hitThis ? hoverScale : defaultScale);
        //if (hitThis)
        //    Logger::Log(U8("難易度選択のモデルに重なっている"));

        if (hitThis && InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
        {
            Logger::Log(U8("難易度選択のモデルを押した"));
            titleScene->SetPhase(TitleScene::TitlePhase::StartWait); // これで板のスケールが変わらないようにsる
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/push_button.wav", 1.0f);
            RequestChangeScene(d.difficulty);
        }
    }
#if 0
    // ① 押した瞬間：選択判定
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        if (hit && result.component == selectModelBoxComponent.get())
        {
            const char* types[] = { "0", "1" };
            //Scene::_transition("LoadingScene", { std::make_pair("preload", "SampleScene"), std::make_pair("type", types[rand() % 2]) });
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty","0"} });
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/grab_ingredient.wav", 1.0f);
            Logger::Log(U8("難易度選択のモデルを押した"));
        }
    }

#endif // 0
}


// 難易度によっての遷移シーン選択
void OdenTitleStageActor::RequestChangeScene(Difficulty diff)
{
    if (diff == Difficulty::Tutorial)
    {
#if 1
        SceneTransitionManager::Instance().RequestTransition(
            "LoadingScene",
            {
                {"preload", "TutorialScene"},
                {"difficulty", "0"}
            }
        );
#else
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "TutorialScene"),{"difficulty", "0"} });
#endif // 0
    }
    else
    {
#if 1
        SceneTransitionManager::Instance().RequestTransition(
            "LoadingScene",
            {
                {"preload", "MainScene"},
                {"difficulty", std::to_string((int)diff)}
            }
        );
#else
        const char* types[] = { "0", "1" };
        Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), {"difficulty", std::to_string(static_cast<int>(diff))} });

#endif // 0

    }
}