#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

class BeatClockActor;
class OdenSlotActor;
class OdenIngredientActor;
class IBeatReactive;


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

    // ゲーム開始時に呼ぶ関数
    void StartGame();

    // 次に来る具材名を取得（UI用）
    std::string GetPreviewIngredient(int index) const;

    // ビートアクターを設定する
    void SetBeatActor(const std::shared_ptr<BeatClockActor>& beatClockActor)
    {
        beatClockWeak = beatClockActor;
    }

    // ビート反応リストを登録する
    void RegisterBeatReactive(const std::shared_ptr<IBeatReactive>& obj)
    {
        beatReactives.push_back(obj);
    }

    // 特定の食材を補充する
    void SupplySpecificIngredientTo(const std::shared_ptr<OdenSlotActor>& slot, const std::string& ingredientName);

private:
    // スロットの回転関数を呼ぶ
    void UpdateBeat(float deltaTime);

    // 空スロットを見つけたら、食材を補充する
    void TrySupplyIngredients();

    // 食材を補充する
    void SupplyIngredientTo(const std::shared_ptr<OdenSlotActor>& slot) ;

    // ランダムな具材の名前を生成する
    std::string MakeRandomIngredientName() ;

    // 食材袋を初期化する
    void BuildIngredientBag();

    // 先にキューを満たす
    void FillIngredientQueue();

    // ビートに合わせてスケールを変更する
    void ApplyBeatScaling(float beatPhase) const;


private:
    static constexpr BeatPattern BeatTable[4] =
    {
        { 0.8f, false }, // ったーん
        { 0.8f, false }, // ったーん
        { 0.8f, false }, // ったーん
        { 0.8f, true  }, // ターン！
    };

    float beatTimer = 0.0f;
    int beatIndex = 0;

    std::vector<std::weak_ptr<OdenSlotActor>> slots;

    std::deque<std::string> ingredientQueue; // 追加される待ちの食材

    static constexpr int previewCount = 3;// 表示用

    std::weak_ptr<BeatClockActor> beatClockWeak; // ビートを刻むアクター

    std::vector<std::weak_ptr<IBeatReactive>> beatReactives; // ビート反応リスト

    std::vector<std::string> ingredientBag; // 具材の袋
    size_t bagIndex = 0; //

    GameDifficulty difficulty = GameDifficulty::Normal;
};