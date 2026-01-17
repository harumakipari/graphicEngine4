#include "pch.h"
#include "OdenOrderManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenData/OdenGameParameter.h"
#include "Utility/GameUtility.h"
#include "OdenGameManager.h"
#include "OdenSlotManager.h"


void OdenOrderManager::Initialize(const Transform& transform)
{
    slots.clear();

    // お題を生成
    for (int i = 0; i < MaxOrders + arrangeOrder; i++) // 奥に見える分
    {
        slots.push_back({  {} ,i});
        SpawnOrderBubble(i);
    }

    RearrangeBubbles();
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

    bubble->onCompleted = [this, index](OdenBubbleActor& bubble, const OdenResult score)
        {
            //　完了時の処理
            OnBubbleCompleted(index,bubble, score);
        };

    slots[index].bubble = bubble;

    // スロットマネージャーに吹き出しを登録する　これで吹き出しがビートに乗る
    auto slotManagerActor = GetOwnerScene()->GetActorManager()->GetActorByName("slotManager");
    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);
    slotManager->RegisterBeatReactive(bubble);
}

// ランダムにお題を生成する
OrderEntry OdenOrderManager::PickRandomOrder()
{
    // あいまいな形のお題の割合
    constexpr float shapeOrderRate = 0.8f;   // 60%

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
void OdenOrderManager::OnBubbleCompleted(int slotIndex,  OdenBubbleActor& bubble, const OdenResult score)
{
    Logger::Log(U8("オーダー完了時のスコア = ") + std::to_string(score.price));

    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            // スコアを加算する
            gameManager->AddScore(score.price);

            // 満足度を加算する
            gameManager->AddSatisfaction(score.satisfaction);
            // リザルト画面のために実際に提出された食材の種類とスコアを記録する
            gameManager->AddSubmitLog(bubble.GetIngredientType(), score.price);
        }
    }

    bubble.SetLeaving();
    slots[slotIndex].bubble.reset();

    // 少し遅らせて新しい客を出すのもアリ
    SpawnOrderBubble(slotIndex);
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

            const bool canOrder = (newIndex < MaxOrders); // ← 3人だけ
     //       b->SetTargetPosition(GetBubblePosition(newIndex), canOrder);

            newIndex++;
        }
    }

    // 後ろに新しい客を補充
    while (newIndex < MaxOrders + arrangeOrder) // 奥に見える分
    {
        SpawnOrderBubble(newIndex);
        newIndex++;
    }
}