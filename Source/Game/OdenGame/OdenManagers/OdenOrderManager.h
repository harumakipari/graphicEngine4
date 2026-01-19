#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"

class OdenBubbleActor;

// お客さんを並べる
class OdenOrderManager :public Actor
{
public:
    explicit OdenOrderManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override {}

private:
    // お客さんを出現させる
    void SpawnOrderBubble(int index);

    // ランダムにお題を生成する
    OrderEntry PickRandomOrder();

    // 順番から位置を取得する
    DirectX::XMFLOAT3 GetBubblePosition(const int index);

    // スロットの番号から出現の位置を取得する
    DirectX::XMFLOAT3 GetBubbleSpawnPosition(const int index);

    // 注文が完了した時に呼ばれる関数
    void OnBubbleCompleted(int slotIndex,  OdenBubbleActor& bubble, OdenResult score);

    // お題のバッグを生成する
    void BuildOrderBag();

    // バッグからお題を取り出す
    OrderEntry PickFromBag();

private:
    struct BubbleSlot
    {
        std::weak_ptr<OdenBubbleActor> bubble;
        int slotIndex;   // 論理的な並び順
    };
    std::vector<BubbleSlot> slots;

    // TODO: 最大の注文を聞ける人数
    static constexpr int MaxOrders = 3; 
    DirectX::XMFLOAT3 basePos = { 2.0f,3.0f,9.0f };
    float spacing = 3.0f;

    float spawnOffsetZ = 3.0f; // 出現時のZオフセット

    // TODO:　注文＋並んでいる人数
    static constexpr int arrangeOrder = 0;

    size_t bagIndex = 0;
    std::vector<OrderEntry> orderBag;

    GameDifficulty difficulty = GameDifficulty::Normal;
};