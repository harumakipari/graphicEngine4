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
    explicit OdenBubbleActor(const std::string& actorName, const std::string& orderUiFileName = "UI_Order_CircleLike") :Actor(actorName), orderUiFileName(orderUiFileName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 食材が落とされたら呼ばれる関数
    void OnIngredientDropped(const OdenIngredientActor& ingredient);

private:
    // スコアを判定する
    EScore JudgeScore(const OdenIngredientActor& ingredient) const;

    // 形からスコアを判定する
    EScore JudgeShapeScore(const OdenShapeData& shape) const;
private:
    std::shared_ptr<UIImageComponent> orderUi; // オーダーの吹き出し
    OrderData orderData = {};    // オーダーのデータ
    std::string orderUiFileName;  // オーダーの名前のUIの.pngの名前

    DirectX::XMFLOAT3 uiOffset = { 0.0f,0.0f,0.0f };   // UIの吹き出し位置のオフセット
};
