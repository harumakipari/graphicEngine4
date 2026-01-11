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
        Leaving  // 去っていく
    };

public:
    explicit OdenBubbleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Finalize() override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // お題を設定する
    void SetOrder(const OrderData& orderData, const std::string& orderUiFileName);

    // 食材が落とされたら呼ばれる関数
    void OnIngredientDropped(const OdenIngredientActor& ingredient);

private:
    // スコアを判定する
    float JudgeScore(const OdenIngredientActor& ingredient) const;

    // 形からスコアを判定する
    float JudgeShapeScore(const OdenShapeData& shape) const;
public:
    std::function<void(OdenBubbleActor&, float score)> onCompleted;     // コールバック関数

private:
    std::shared_ptr<UIImageComponent> orderUi; // オーダーの吹き出し
    OrderData orderData = {};    // オーダーのデータ
    std::string orderUiFileName;  // オーダーの名前のUIの.pngの名前

    DirectX::XMFLOAT3 uiOffset = { 0.0f,0.0f,0.0f };   // UIの吹き出し位置のオフセット

    EBubbleState state = EBubbleState::Waiting; // 状態
};
