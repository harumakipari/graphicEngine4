#include "pch.h"
#include "OdenOrderManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenData/OdenGameParameter.h"
#include "Utility/GameUtility.h"
#include "OdenGameManager.h"


void OdenOrderManager::Initialize(const Transform& transform)
{
    slots.clear();

    // お題を生成
    for (int i = 0; i < MaxOrders + 2; i++) // 奥に見える分
    {
        SpawnOrderBubble(i);
    }
}


// お客さんを出現させる
void OdenOrderManager::SpawnOrderBubble(int index)
{
    // 位置を取得する
    Transform tr(
        GetBubblePosition(index),
        { 0,0,0,1 },
        { 1,1,1 }
    );

    // お題アクターを生成する
    auto bubble =
        Scene::GetCurrentScene()->GetActorManager()->
        CreateAndRegisterActorWithTransform<OdenBubbleActor>("OdenBubble", tr);

    // お題をランダムに生成する
    OrderEntry randomOrder = PickRandomOrder();

    // お題を設定する
    bubble->SetOrderAndMakeUi(randomOrder.data, randomOrder.uiName);

    bubble->onCompleted = [this](const OdenBubbleActor& bubble, const float score)
        {
            //　完了時の処理
            OnBubbleCompleted(bubble, score);
        };

    BubbleSlot slot;
    slot.bubble = bubble;
    slot.slotIndex = index;

    slots.push_back(slot);
}

// ランダムにお題を生成する
OrderEntry OdenOrderManager::PickRandomOrder()
{
    // あいまいな形のお題の割合
    constexpr float shapeOrderRate = 0.6f;   // 60%

    float random = MathHelper::RandomRange(0.0f, 1.0f);

    if (random < shapeOrderRate)
    {
        // 形指定お題
        return GameHelper::PickRandom(OdenGameParameter::orderDB.shapeOrders);
    }
    // 食材指定お題
    return GameHelper::PickRandom(OdenGameParameter::orderDB.ingredientOrders);
}

// 順番から位置を取得する
DirectX::XMFLOAT3 OdenOrderManager::GetBubblePosition(const int index)
{
    return
    {
        basePos.x + spacing * static_cast<float>(index),
        basePos.y,
        basePos.z
    };
}

// 注文が完了した時に呼ばれる関数
void OdenOrderManager::OnBubbleCompleted(const OdenBubbleActor& bubble, const float score)
{
    Logger::Log(U8("オーダー完了時のスコア = ") + std::to_string(score));

    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            gameManager->AddScore(score);
        }
    }

    // 列から外す
    for (auto& s : slots)
    {
        if (auto b = s.bubble.lock())
        {
            if (b.get() == &bubble)
            {
                b->SetLeaving();
                s.bubble.reset(); // スロットは空に
                break;
            }
        }
    }

    // スロット再配置（詰める）
    RearrangeBubbles();
}

// 並びを詰める
void OdenOrderManager::RearrangeBubbles()
{
    int newIndex = 0;

    for (auto& s : slots)
    {
        if (auto b = s.bubble.lock())
        {
            s.slotIndex = newIndex;

            b->SetTargetPosition(GetBubblePosition(newIndex));

            newIndex++;
        }
    }

    // 後ろに新しい客を補充
    while (newIndex < MaxOrders + 2) // 奥に見える分
    {
        SpawnOrderBubble(newIndex);
        newIndex++;
    }
}