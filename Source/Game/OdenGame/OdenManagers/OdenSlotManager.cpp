#include "pch.h"
#include "OdenSlotManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenSlotActor.h"
#include "Game/OdenGame/OdenIngredientActor.h"

// 初期化
void OdenSlotManager::Initialize(const Transform& transform)
{
#if 0
    // 下4段  横回転
    for (int i = 0; i < 4; ++i)
    {
        // おでんのダイコンを生成
        Transform daikonTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = Scene::GetCurrentScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenDaikonActor>("Daikon", daikonTr);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,0.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto slot = Scene::GetCurrentScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotActor>("odenSlot_Horizontal", odenSlotTr);
        slot->rotationType = ERotationType::Horizontal;
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);

        slots.push_back(slot);
    }

    // 上4段  縦回転
    for (int i = 0; i < 4; ++i)
    {
        // おでんのこんにゃくを生成
        Transform konnyakuTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto ingredient = Scene::GetCurrentScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenKonnyakuActor>("Daikon", konnyakuTr);

        // スロット生成
        Transform odenSlotTr(DirectX::XMFLOAT3{ i * 4.0f,1.0f,4.0f }, DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f });
        auto slot = Scene::GetCurrentScene()->GetActorManager()->CreateAndRegisterActorWithTransform<OdenSlotActor>("odenSlot_Vertical", odenSlotTr);
        slot->rotationType = ERotationType::Vertical;
        slot->SetIngredient(ingredient);
        ingredient->SetCurrentSlot(slot);

        slots.push_back(slot);
    }

#endif // 0
}


// スロットの回転関数を呼ぶ
void OdenSlotManager::Update(float deltaTime)
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
