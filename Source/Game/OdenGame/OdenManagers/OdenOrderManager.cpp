#include "pch.h"
#include "OdenOrderManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenBubbleActor.h"
#include "Utility/GameUtility.h"


void OdenOrderManager::Initialize(const Transform& transform)
{
    // お題を生成
    for (int i = 0; i < 3; i++)
    {
        // 位置を取得する
        Transform tr(
            GetBubblePosition(i),
            { 0,0,0,1 },
            { 1,1,1 }
        );

        // お題アクターを生成する
        auto bubble =
            GetOwnerScene()->GetActorManager()->
            CreateAndRegisterActorWithTransform<OdenBubbleActor>("OdenBubble", tr);

        // お題をランダムに生成する
        OrderEntry randomOrder = PickRandomOrder();

        // お題を設定する
        bubble->SetOrder(randomOrder.data, randomOrder.uiName);

        bubbles.push_back(bubble);

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
    bubble->SetOrder(randomOrder.data, randomOrder.uiName);

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
        basePos.x + spacing * index,
        basePos.y,
        basePos.z
    };
}

