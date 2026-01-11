#include "pch.h"
#include "OdenOrderManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenBubbleActor.h"
#include "Utility/GameUtility.h"


void OdenOrderManager::Initialize(const Transform& transform)
{
    // お題を生成
    for (int i = 0; i < MaxOrders; i++)
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


    bubbles.push_back(bubble);
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
        return GameHelper::PickRandom(OrderDB.shapeOrders);
    }
    // 食材指定お題
    return GameHelper::PickRandom(OrderDB.ingredientOrders);
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

    // 列から外す
    auto it = std::remove_if(bubbles.begin(), bubbles.end(), [&](const std::weak_ptr<OdenBubbleActor>& w)
        {
            auto b = w.lock();
            return !b || b.get() == &bubble;
        });

    bubbles.erase(it, bubbles.end());

    // 並びを詰める
    RearrangeBubbles();

    // 新しい客を最後尾に生成する
    SpawnOrderBubble(static_cast<int>(bubbles.size()));
}

// 並びを詰める
void OdenOrderManager::RearrangeBubbles()
{
    for (int i = 0; i < bubbles.size(); ++i)
    {
        if (auto bubble = bubbles[i].lock())
        {
            // 一旦瞬間移動
            bubble->SetPosition(GetBubblePosition(i));
        }
    }
}