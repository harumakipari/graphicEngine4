#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "UI/Widgets/Widget.h"

// オーダーのタイプ
enum class EOrderType
{
    ShapeOnly,
    SpecificIngredient
};


struct OrderData
{
    EOrderType type;

    // Shape系
    EOdenShapeCategory requiredCategory;
    ShapeProperty targetProperty;

    // 具材指定系
    EOdenType requiredIngredient;

    float timeLimit;
    float remainingTime;
};

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
    explicit OdenBubbleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

    // スコアを判定する
    EScore JudgeScore(const OdenIngredientActor& ingredient) const;

private:
    // 形からスコアを判定する
    EScore JudgeShapeScore(const OdenShapeData& shape) const;
private:
    std::shared_ptr<UIImageComponent> orderUi; // オーダーの吹き出し
    OrderData order;    // オーダーのデータ
};
