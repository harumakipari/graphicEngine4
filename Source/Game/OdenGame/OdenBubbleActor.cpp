#include "pch.h"
#include "OdenBubbleActor.h"

#include <magic_enum.hpp>

#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenIngredientActor.h"
#include "OdenData/OdenShapeDataTable.h"

void OdenBubbleActor::Initialize(const Transform& transform)
{
    std::string parentName = "OrderBubble_Box";

    // 当たり判定を登録
    auto boxComponent = AddComponent<BoxComponent>(parentName);
    DirectX::XMFLOAT3 size = { 5.0f,5.0f,5.0f };
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);// おでんのゲームのカーソルのターゲット
    boxComponent->Initialize();

    // ここでお題のデータを入れる
    orderData = gameOdenOrderData.odenOrders[orderUiFileName];

    // お題のUIコンポーネントを作成する
    std::string filename = "./Data/Textures/UI/" + orderUiFileName + ".png";
    orderUi = std::make_shared<UIImageComponent>(filename, "OdenBubbleUi");
    orderUi->SetWorldPosition({ 50, 300 });
    orderUi->SetPivot({ 0.5f,0.5f });
    orderUi->SetSize({ 150, 200 });
    GetOwnerScene()->GetUIManager()->Add(orderUi);

}

void OdenBubbleActor::Update(float elapsedTime)
{
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x + uiOffset.x, position.y + uiOffset.y, position.z + uiOffset.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);
    orderUi->SetWorldPosition({ uiPos.x, uiPos.y });

}

void OdenBubbleActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI

    ImGui::Text("UI File : %s", orderUiFileName.c_str());

    ImGui::Separator();
    ImGui::Text("== Order Data ==");

    // OrderType
    ImGui::Text("Order Type : %s",
        magic_enum::enum_name(orderData.type).data());

    // ShapeOnly
    if (orderData.type == EOrderType::ShapeOnly)
    {
        ImGui::Text("Required Shape Category : %s",
            magic_enum::enum_name(orderData.requiredCategory).data());

        //DrawShapeProperty(orderData.targetProperty);
    }

    // SpecificIngredient
    if (orderData.type == EOrderType::SpecificIngredient)
    {
        ImGui::Text("Required Ingredient : %s",
            magic_enum::enum_name(orderData.requiredIngredient).data());
    }

    ImGui::Separator();
    ImGui::Text("== Timer ==");
    ImGui::Text("Time Limit     : %.1f", orderData.timeLimit);
    ImGui::Text("Remaining Time : %.1f", orderData.remainingTime);

    ImGui::DragFloat3(U8("UIの吹き出し位置のオフセット"), &uiOffset.x, 0.5f);

#endif
};

// 食材が落とされたら呼ばれる関数
void OdenBubbleActor::OnIngredientDropped(const OdenIngredientActor& ingredient)
{
    EScore score = JudgeScore(ingredient);

    // ここでスコアのフォントを

    // ゲーム全体へ通知

}


EScore OdenBubbleActor::JudgeScore(const OdenIngredientActor& ingredient) const
{
    const OrderData& o = orderData;

    // 具材指定のお題
    if (o.type == EOrderType::SpecificIngredient)
    {
        if (ingredient.GetIngredientType() == o.requiredIngredient)
        {
            Logger::Log(U8("具材指定のお題　パーフェクト！"));
            return EScore::Perfect;
        }
        else
        {
            Logger::Log(U8("具材指定のお題　失敗(T_T)"));
            return EScore::Fail;
        }
    }

    // Shape系お題
    if (o.type == EOrderType::ShapeOnly)
    {
        return JudgeShapeScore(ingredient.GetCurrentShape());
    }

    return EScore::Fail;
}

EScore OdenBubbleActor::JudgeShapeScore(const OdenShapeData& shape) const
{
    // カテゴリが違うと
    if (shape.category != orderData.requiredCategory)
    {
        Logger::Log(U8("あいまいな形指定のお題　失敗(T_T)"));
        return EScore::Fail;
    }

    float dr = fabs(shape.property.roundness - orderData.targetProperty.roundness);
    float da = fabs(shape.property.aspectRatio - orderData.targetProperty.aspectRatio);

    float dist = dr + da;

    if (dist < 0.1f)
    {
        Logger::Log(U8("あいまいな形指定のお題　パーフェクト！"));
        return EScore::Perfect;
    }
    if (dist < 0.3f)
    {
        Logger::Log(U8("あいまいな形指定のお題　Great！"));
        return EScore::Great;
    }
    if (dist < 0.6f)
    {
        Logger::Log(U8("あいまいな形指定のお題　Good！"));
        return EScore::Good;
    }
    Logger::Log(U8("あいまいな形指定のお題　失敗(T_T)"));
    return EScore::Fail;
}
