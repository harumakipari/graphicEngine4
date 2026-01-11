#include "pch.h"
#include "OdenSlotManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenSlotActor.h"
#include "Game/OdenGame/OdenDetailIngredientsActors.h"
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
    // 下4段  横回転
    for (int i = 0; i < 4; ++i)
    {
        // ランダムに名前を選択
        const std::string& selectedName = MakeRandomIngredientName();

        // おでんの具材を生成
        Transform daikonTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", daikonTr, selectedName);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
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

        // おでんの具材を生成
        Transform konnyakuTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", konnyakuTr, selectedName);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto slot = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotActor>("odenSlot_Vertical", odenSlotTr);
        slot->rotationType = ERotationType::Vertical;
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);

        RegisterSlot(slot);
    }
}

// スロットの回転関数を呼ぶ
void OdenSlotManager::UpdateBeat(float deltaTime)
{
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
    auto actorManager = Scene::GetCurrentScene()->GetActorManager();


    // ランダムに名前を選択
    const std::string& selectedName = MakeRandomIngredientName();

    // おでんの具材を生成
    Transform ingredientTr(DirectX::XMFLOAT3{ 0.0f,1.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
    auto ingredient = actorManager->CreateAndRegisterActorWithTransform<OdenIngredientActor>("OdenIngredient", ingredientTr, selectedName);
    ingredient->SetPosition(slot->GetPosition());
    ingredient->SetCurrentSlot(slot);
    slot->SetIngredient(ingredient);
}

// ランダムな具材の名前を生成する
std::string OdenSlotManager::MakeRandomIngredientName()
{
    // 生成可能な具材名のリスト
    static const std::vector<std::string> ingredientNames = {
        "Daikon",
        "Egg",
        "Tsukune",
        "Chikuwa",
        "Konnyaku"
        // ここに追加していく
    };

    // ランダムに選択
    const std::string& selectedName = GameHelper::PickRandom(ingredientNames);

    return selectedName;
}