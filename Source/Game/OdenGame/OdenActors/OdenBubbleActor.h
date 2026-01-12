#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "UI/Widgets/Widget.h"


enum class EScore :uint8_t
{
    Perfect,
    Great,
    Good,
    Fail,
};

class OdenIngredientActor;

// 　お題
// 　ふきだし
//
class OdenBubbleActor :public Actor
{
public:
    // お題の状態
    enum class EBubbleState :uint8_t
    {
        Waiting, // 待っている
        Leaving,  // 去っていく
        QueuingMove     // 列の詰め動作中
    };

public:
    explicit OdenBubbleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Finalize() override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // お題を設定する　Uiを生成する
    void SetOrderAndMakeUi(const OrderData& orderData, const std::string& orderUiFileName);

    // 食材が落とされたら呼ばれる関数
    void OnIngredientDropped(const OdenIngredientActor& ingredient);

private:
    // 判定を判定する
    float JudgeMatchShapeRate(const OdenIngredientActor& ingredient) const;

    // 形からスコアを判定する
    float JudgeShapeScore(const OdenShapeData& shape) const;

    // 値段と合わせたスコアを計算する
    float CalculateOrderScore(const OdenIngredientActor& ingredient, float matchRate, float multiplier);

public:
    std::function<void(OdenBubbleActor&, float score)> onCompleted;     // コールバック関数

private:
    std::shared_ptr<UIImageComponent> orderUi; // オーダーの吹き出し
    OrderData orderData = {};    // オーダーのデータ
    std::string orderUiFileName;  // オーダーの名前のUIの.pngの名前

    DirectX::XMFLOAT2 uiOffset = { 0.0f,-2.0f };   // UIのゲージ位置のオフセット

    EBubbleState state = EBubbleState::Waiting; // 状態

    float remainingTime = 0.0f; // この Bubble の残り時間
    float timeLimit = 0.0f;     // この Bubble の制限時間
    std::shared_ptr<UIGaugeComponent> gaugeUi; // 残り時間のゲージUI
    std::shared_ptr<UITextPopup> scorePopupUi; // 取得したスコアを表示するUI

};
