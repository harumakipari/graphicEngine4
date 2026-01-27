#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"

class OdenSlotManager;
class OdenBubbleActor;

// お客さんを並べる
class OdenOrderManager :public Actor
{
public:
    explicit OdenOrderManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override {}

    void StartGame();

    // 特定のお題を出現させる
    void SpawnSpecificOrderBubble(int index, const std::string& uiName);

    // スロットマネージャーを設定する
    void SetSlotManager(const std::shared_ptr<OdenSlotManager>& slotManager);

private:
    // お客さんを出現させる
    void SpawnOrderBubble(int index);

    // ランダムにお題を生成する
    OrderEntry PickRandomOrder();

    // 順番から位置を取得する
    DirectX::XMFLOAT3 GetBubblePosition(const int index);

    // スロットの番号から出現の位置を取得する
    DirectX::XMFLOAT3 GetBubbleSpawnPosition(const int index);

    // お題のバッグを生成する
    void BuildOrderBag();

    // 注文が完了した時に呼ばれる関数
    void OnBubbleCompleted(int slotIndex, OdenBubbleActor& bubble, OdenResult score);

    // --- チュートリアル用　 --- //
    // 注文が完了した時に呼ばれる関数 チュートリアル用
    void OnBubbleCompletedTutorial(int slotIndex, OdenBubbleActor& bubble, OdenResult score);

    // UI名からお題を探す
    const OrderEntry* FindOrderByUiName(const std::string& uiName);

    // 出せる特定の食材お題だけを抽出する
    std::vector<OrderEntry> GetValidIngredientOrders() const;

    // バッグ × 場に存在する で絞る
    std::vector<OrderEntry> GetValidIngredientOrdersFromBag() const;
private:
    struct BubbleSlot
    {
        std::weak_ptr<OdenBubbleActor> bubble;
        int slotIndex;   // 論理的な並び順
    };
    std::vector<BubbleSlot> slots;

    // TODO: 最大の注文を聞ける人数
    static constexpr int MaxOrders = 3;
    DirectX::XMFLOAT3 basePos = { -0.9f,1.2f,10.0f };
    float spacing = 6.1f;

    float spawnOffsetZ = 3.0f; // 出現時のZオフセット

    // TODO:　注文＋並んでいる人数
    static constexpr int arrangeOrder = 0;

    size_t bagIndex = 0;
    std::vector<OrderEntry> orderBag;

    GameDifficulty difficulty = GameDifficulty::Normal;

    std::weak_ptr<OdenSlotManager> slotManagerWeak;

    std::vector<OrderEntry> shapeOrderBag; // あいまいなお題用
    std::vector<OrderEntry> ingredientOrderBag; // 特定の食材のお題用

    int shapeBagIndex = 0;
    int ingredientBagIndex = 0;

    int shapeChainCount = 0;
};