#pragma once
#include "Core/Actor.h"

class OdenSlotActor;
class OdenIngredientActor;



class OdenSlotManager :public Actor
{
public:
    // ビートパターン
    struct BeatPattern
    {
        float interval;   // 拍の間隔（秒）
        bool strong;      // 強拍かどうか
    };
public:
    OdenSlotManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // スロットを登録する
    void RegisterSlot(const std::shared_ptr<OdenSlotActor>& slot)
    {
        slots.push_back(slot);
    }

private:
    // スロットの回転関数を呼ぶ
    void UpdateBeat(float deltaTime);

    // 空スロットを見つけたら、食材を補充する
    void TrySupplyIngredients();

    // 食材を補充する
    void SupplyIngredientTo(const std::shared_ptr<OdenSlotActor>& slot);

    // ランダムな具材の名前を生成する
    std::string MakeRandomIngredientName();
private:
    static constexpr BeatPattern BeatTable[4] =
    {
        { 1.4f, false }, // ったーん
        { 1.4f, false }, // ったーん
        { 1.4f, false }, // ったーん
        { 1.6f, true  }, // ターン！
    };

    float beatTimer = 0.0f;
    int beatIndex = 0;

    std::vector<std::weak_ptr<OdenSlotActor>> slots;
};