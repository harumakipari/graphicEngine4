#include "pch.h"
#include "OdenBubbleActor.h"
#include "Game/OdenGame/OdenIngredientActor.h"

void OdenBubbleActor::Initialize(const Transform& transform)
{
    std::string parentName = "OrderBubble_Box";

    // 当たり判定を登録
    auto boxComponent = AddComponent<BoxComponent>(parentName);
    DirectX::XMFLOAT3 size ={5.0f,5.0f,5.0f};
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);// おでんのゲームのカーソルのターゲット
    boxComponent->Initialize();

}

EScore OdenBubbleActor::JudgeScore(const OdenIngredientActor& ingredient) const
{
    const OrderData& o = order;

    // 具材指定のお題
    if (o.type == EOrderType::SpecificIngredient)
    {
        if (ingredient.GetIngredientType() == o.requiredIngredient)
            return EScore::Perfect;
        else
            return EScore::Fail;
    }

    // Shape系お題
    if (o.type == EOrderType::ShapeOnly)
    {
        //return JudgeShapeScore(ingredient.GetCurrentShape());
    }

    return EScore::Fail;
}

EScore OdenBubbleActor::JudgeShapeScore(const OdenShapeData& shape) const
{
    // カテゴリが違うと
    if (shape.category != order.requiredCategory)
        return EScore::Fail;

    float dr = fabs(shape.property.roundness - order.targetProperty.roundness);
    float da = fabs(shape.property.aspectRatio - order.targetProperty.aspectRatio);

    float dist = dr + da;

    if (dist < 0.1f) return EScore::Perfect;
    if (dist < 0.3f) return EScore::Great;
    if (dist < 0.6f) return EScore::Good;

    return EScore::Fail;
}
