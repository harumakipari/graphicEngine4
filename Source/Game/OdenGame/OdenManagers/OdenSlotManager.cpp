#include "pch.h"
#include "OdenSlotManager.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"
#include "Game/OdenGame/OdenActors/OdenDetailIngredientsActors.h"
#include "Game/OdenGame/BeatClockActor.h"
#include "Game/OdenGame/OdenGameSession.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"
#include "Utility/GameUtility.h"

// 初期化
void OdenSlotManager::Initialize(const Transform& transform)
{
}


void OdenSlotManager::Update(float deltaTime)
{
    // ビート処理
    UpdateBeat(deltaTime);

    // 空スロットを見つけたら、食材を補充する
    TrySupplyIngredients();
}

// ゲーム開始時に呼ぶ関数
void OdenSlotManager::StartGame()
{
    // 最初にキューを作成
    FillIngredientQueue();


    // 下4段  横回転
    for (int i = 0; i < 4; ++i)
    {
        // ランダムに名前を選択
        const std::string& selectedName = MakeRandomIngredientName();
        //const std::string& selectedName = "Konnyaku";

        // おでんの具材を生成
        Transform downIngredientTr(DirectX::XMFLOAT3{ i * 4.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", downIngredientTr, selectedName);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,0.0f }, XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto slot = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotActor>("odenSlot_Horizontal", odenSlotTr);
        slot->rotationType = ERotationType::Horizontal;
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);

        RegisterSlot(slot);
    }

    // 上4段  縦回転
    for (int i = 0; i < 4; ++i)
    {
        // ランダムに名前を選択
        const std::string& selectedName = MakeRandomIngredientName();
        //const std::string& selectedName = "Daikon";

        // おでんの具材を生成
        Transform odenUpIngredient(DirectX::XMFLOAT3{ i * 4.0f,0.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", odenUpIngredient, selectedName);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto slot = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotActor>("odenSlot_Vertical", odenSlotTr);
        slot->rotationType = ERotationType::Vertical;
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);

        RegisterSlot(slot);
    }
}

// 次に来る具材名を取得（UI用）
std::string OdenSlotManager::GetPreviewIngredient(const int index) const
{
    if (index < 0 || index >= ingredientQueue.size())
        return "";

    return ingredientQueue[index];
}

// スロットの回転関数を呼ぶ
void OdenSlotManager::UpdateBeat(float deltaTime)
{
#if 0
    auto clock = beatClockWeak.lock();
    if (!clock)
        return;

#if 1
    int beatCount = clock->ConsumeAdvancedBeatCount();

    for (int i = 0; i < beatCount; ++i)
    {
        for (auto& slot : slots)
        {
            if (auto slotActor = slot.lock())
            {
                slotActor->OnBeat();
            }
        }

    }

#endif // 0

    // 拍が切り替わった瞬間
    if (clock->ConsumeBeatJustChanged())
    {
        const auto& beat = clock->GetCurrentBeat();

        // 回転音　SE再生
        //CoreAudio::PlayOneShot(L"./Data/Sound/SE/turning.wav", 0.3f);
        //CoreAudio::PlayOneShot(L"./Data/Sound/SE/turning_finger_clap.wav");
        // スロットの食材回転処理
#if 0
        for (auto& slot : slots)
        {
            if (auto slotActor = slot.lock())
            {
                slotActor->OnBeat(/*beat.isStrong*/);
            }
        }

#endif // 0

        // 拍が切り替わった時の処理
        for (auto& beatReact : beatReactives)
        {
            if (auto r = beatReact.lock())
            {
                r->OnBeat(beat.isStrong);
            }
        }
    }

    // ビートに合わせて0~1の数値を送る
    float phase = clock->GetBeatPhase();
    //phase = 1.0f;
    for (auto& beatReact : beatReactives)
    {
        if (auto r = beatReact.lock())
        {
            r->OnBeatPhase(phase);
        }
    }

    // ビートに合わせたスケール処理
    ApplyBeatScaling(phase);

#else
    beatTimer += deltaTime;

    const auto& beat = BeatTable[beatIndex];

    if (beatTimer >= beat.interval)
    {
        beatTimer -= beat.interval;

        // ビート
        for (auto& slot : slots)
        {
            if (auto slotActor = slot.lock())
            {
                slotActor->OnBeat(/*beat.strong*/);
            }
        }

        beatIndex = (beatIndex + 1) % 4;
    }
#endif // 0

}

// 空スロットを見つけたら、食材を補充する
void OdenSlotManager::TrySupplyIngredients()
{
    // ビート
    for (auto& slot : slots)
    {
        auto slotActor = slot.lock();
        if (!slotActor)
            continue;

        if (!slotActor->GetIngredient())
        {
            SupplyIngredientTo(slotActor);
        }
    }
}

// 食材を補充する
void OdenSlotManager::SupplyIngredientTo(const std::shared_ptr<OdenSlotActor>& slot)
{
    if (ingredientQueue.empty())
        FillIngredientQueue();

    // 先頭を取り出す
    std::string ingredientName = ingredientQueue.front();
    ingredientQueue.pop_front();
    auto actorManager = Scene::GetCurrentScene()->GetActorManager();


    // おでんの具材を生成
    Transform ingredientTr(DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto ingredient = actorManager->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", ingredientTr, ingredientName);
    XMFLOAT3 slotPosition = slot->GetPosition();
    slotPosition.y -= 1.0f;
    ingredient->SetPosition(slotPosition);

    ingredient->SetCurrentSlot(slot);
    slot->SetIngredient(ingredient);

    // 次のために補充
    FillIngredientQueue();
}

// ランダムな具材の名前を生成する
std::string OdenSlotManager::MakeRandomIngredientName() 
{
    // Bagが空 or 使い切ったら再生成
    if (ingredientBag.empty() || bagIndex >= ingredientBag.size())
    {
        BuildIngredientBag();
    }

    std::string result = ingredientBag[bagIndex++];

    // 直前と同じなら1回だけスキップ（保険）
    if (!ingredientQueue.empty() &&
        result == ingredientQueue.back() &&
        bagIndex < ingredientBag.size())
    {
        result = ingredientBag[bagIndex++];
    }

    return result;


    // 生成可能な具材名のリスト
    static const std::vector<std::string> ingredientNames = {
        "Daikon",
        "Egg",
        "Tsukune",
        "Chikuwa",
        "Konnyaku",
        "Hanpen",
        "Goboten",
        "Cake",
        "Donut",
        "Shirataki",
        "Kobumusubi"
        // ここに追加していく
    };

    // ランダムに選択
    std::string candidate;
    do
    {
        candidate = GameHelper::PickRandom(ingredientNames);
    } while (!ingredientQueue.empty() &&
        ingredientQueue.back() == candidate);

    return candidate;
}

// 食材袋を初期化する
void OdenSlotManager::BuildIngredientBag()
{
    struct IngredientEntry
    {
        std::string name;
        int count;
    };

    std::vector<IngredientEntry> ingredients;

    difficulty = OdenGameSession::GetDifficulty();

    if (difficulty == GameDifficulty::Easy)
    {
        ingredients =
        {
            { "Daikon", 1 },
            //{ "Egg", 1},
            //{ "Tsukune", 1 },
            //{ "Chikuwa", 1 },
            //{ "Konnyaku", 1 },
            //{ "Hanpen", 1 },
            //{ "Cake", 1 },
            //{ "Donut", 1 },
        };
    }
    else if (difficulty==GameDifficulty::Hard)
    {
        ingredients =
        {
            { "Daikon", 1 },
            { "Egg", 1},
            { "Tsukune", 1 },
            { "Chikuwa", 1 },
            { "Konnyaku", 1 },
            { "Hanpen", 1 },
            { "Goboten", 1 },
            { "Shirataki", 1 },
            { "Kobumusubi", 1 },
            { "Cake", 1 },
            { "Donut", 1 },
        };
    }


    ingredientBag.clear();

    for (const auto& e : ingredients)
    {
        for (int i = 0; i < e.count; ++i)
        {
            ingredientBag.push_back(e.name);
        }
    }

    GameHelper::Shuffle(ingredientBag);
    bagIndex = 0;
}

// 先にキューを満たす
void OdenSlotManager::FillIngredientQueue()
{
    while (ingredientQueue.size() < previewCount + 8) // 余裕を持つ
    {
        ingredientQueue.push_back(MakeRandomIngredientName());
    }
}

// ビートに合わせてスケールを変更する
void OdenSlotManager::ApplyBeatScaling(float beatPhase) const
{
    float pulse = sinf(beatPhase * DirectX::XM_2PI);
    float scale = 1.0f + pulse * 0.05f;

    for (auto& slot : slots)
    {
        if (auto slotActor = slot.lock())
        {
            slotActor->SetVisualScale(scale);
        }
    }
}
