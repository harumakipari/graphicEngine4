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
    DirectX::XMFLOAT3 size = { 2.5f,5.0f,5.0f };
    boxComponent->SetBoxExtent(size);
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::OdenHoverTarget);// おでんのゲームのカーソルのターゲット
    boxComponent->Initialize();

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

// お題を設定する
void OdenBubbleActor::SetOrder(const OrderData& orderData, const std::string& orderUiFileName)
{
    // ここでお題のデータを入れる
    this->orderData = orderData;

    // お題のUIコンポーネントを作成する
    std::string filename = "./Data/Textures/UI/" + orderUiFileName + ".png";
    orderUi = std::make_shared<UIImageComponent>(filename, "OdenBubbleUi");
    orderUi->SetWorldPosition({ 50, 300 });
    orderUi->SetPivot({ 0.5f,0.5f });
    orderUi->SetSize({ 200, 150 });
    GetOwnerScene()->GetUIManager()->Add(orderUi);

}

// 食材が落とされたら呼ばれる関数
void OdenBubbleActor::OnIngredientDropped(const OdenIngredientActor& ingredient) const
{
    float score = JudgeScore(ingredient);

    // ここでスコアのフォントを

    // ゲーム全体へ通知

}


float OdenBubbleActor::JudgeScore(const OdenIngredientActor& ingredient) const
{
    const OrderData& o = orderData;

    // 具材指定のお題
    if (o.type == EOrderType::SpecificIngredient)
    {
        if (ingredient.GetIngredientType() == o.requiredIngredient)
        {
            Logger::Log(U8("具材指定のお題　パーフェクト！"));
            return 100.0f;
        }
        else
        {
            Logger::Log(U8("具材指定のお題　失敗(T_T)"));
            return 0.0f;
        }
    }

    // Shape系お題
    if (o.type == EOrderType::ShapeOnly)
    {
        float percent = JudgeShapeScore(ingredient.GetCurrentShape());
        Logger::Log(U8("あいまいな形指定のお題") + std::to_string(percent) + U8("パーセント！"));
        return percent;
    }

    Logger::Warning(U8("JudgeScoreに何にも属していないのが来た"));
    return 0.0f;
}

#if 1
float OdenBubbleActor::JudgeShapeScore(const OdenShapeData& shape) const
{
    if (shape.category != orderData.requiredCategory)
    {
        return 0.0f;
    }

    // 丸みの違い
    float dr = fabs(shape.property.roundness - orderData.targetProperty.roundness);
    // 縦横比の違い
    float da = fabs(shape.property.aspectRatio - orderData.targetProperty.aspectRatio);
    // 穴が開いているかどうか
    float dh = fabs(shape.property.holeNess - orderData.targetProperty.holeNess);

    // 理想の形からどれくらいずれているか
    float roundValue = 1.0f;
    float aspectValue = 0.8f;
    float holeValue = 0.3f;

    float dist = dr * roundValue + da * aspectValue + dh * holeValue;

    const float maxDist = static_cast<float>(1.0f * roundValue + 1.0 * aspectValue + 1.0f * holeValue);

    float matchRate = 1.0f - (dist / maxDist);
    matchRate = std::clamp(matchRate, 0.0f, 1.0f);

    return matchRate * 100.0f;
}
#else
EScore OdenBubbleActor::JudgeShapeScore(const OdenShapeData& shape) const
{
    // カテゴリが違うと
    if (shape.category != orderData.requiredCategory)
    {
        Logger::Log(U8("あいまいな形指定のお題　失敗(T_T)"));
        return EScore::Fail;
    }

    // 丸みの違い
    float dr = fabs(shape.property.roundness - orderData.targetProperty.roundness);
    // 縦横比の違い
    float da = fabs(shape.property.aspectRatio - orderData.targetProperty.aspectRatio);
    // 穴が開いているかどうか
    float dh = fabs(shape.property.holeNess - orderData.targetProperty.holeNess);

    // 理想の形からどれくらいずれているか
    float dist = dr * 1.0f + da * 0.8f + dh * 0.3f;

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
#endif // 0

