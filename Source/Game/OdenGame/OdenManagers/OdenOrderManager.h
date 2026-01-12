#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"

class OdenBubbleActor;

// お客さんを並べる
class OdenOrderManager :public Actor
{
public:
    OdenOrderManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override {}

private:
    // お客さんを出現させる
    void SpawnOrderBubble(int index);

    // ランダムにお題を生成する
    OrderEntry PickRandomOrder();

    // 順番から位置を取得する
    DirectX::XMFLOAT3 GetBubblePosition(const int index);

    // 注文が完了した時に呼ばれる関数
    void OnBubbleCompleted(const OdenBubbleActor& bubble, float score);

    // 並びを詰める
    void RearrangeBubbles();
private:
    struct BubbleSlot
    {
        std::weak_ptr<OdenBubbleActor> bubble;
        int slotIndex;   // 論理的な並び順
    };
    std::vector<BubbleSlot> bubbleSlots;

    std::vector<std::weak_ptr<OdenBubbleActor>> bubbles;

    // 最大の注文を聞ける人数
    static constexpr int MaxOrders = 3;
    DirectX::XMFLOAT3 basePos = { 2.0f,3.0f,9.0f };
    float spacing = 3.0f;

};