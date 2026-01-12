#include "pch.h"
#include "OdenBubbleActor.h"

#include <magic_enum.hpp>

#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"

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

#if 1
    // 取得したスコアを表示するUI
    scorePopupUi = std::make_shared<UITextPopup>("OdenScorePopupUi");
    scorePopupUi->SetWorldPosition({ 50, 300 });
    scorePopupUi->SetVisible(false);
    GetOwnerScene()->GetUIManager()->Add(scorePopupUi);

#endif // 0
    state = EBubbleState::Waiting;
}

void OdenBubbleActor::Finalize()
{
    if (orderUi)
    {// 削除通知を出す
        orderUi->MarkPendingKill();
    }
    if (gaugeUi)
    {
        gaugeUi->MarkPendingKill();
    }
}

void OdenBubbleActor::Update(float elapsedTime)
{
    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    if (orderUi)
        orderUi->SetWorldPosition({ uiPos.x, uiPos.y });

    if (scorePopupUi)
        scorePopupUi->SetWorldPosition({ uiPos.x, uiPos.y });

    XMFLOAT2 gaugeUiPos = { uiPos.x + uiOffset.x,uiPos.y + uiOffset.y };

    if (gaugeUi)
    {
        gaugeUi->SetValue(remainingTime, timeLimit);
        gaugeUi->SetWorldPosition({ gaugeUiPos.x, gaugeUiPos.y });
    }

#if 0 // ゲージで去っていく処理　デバック中やりにくいから一旦コメントアウト
    // 待機
    if (state == EBubbleState::Waiting)
    {
        // 残り時間を減らす
        remainingTime -= elapsedTime;

        if (remainingTime <= 0.0f)
        {
            remainingTime = 0.0f;
            state = EBubbleState::Leaving;

            Logger::Log(U8("時間切れで自動失敗"));

            // スコア 0 で通知
            if (onCompleted)
                onCompleted(*this, 0.0f);
        }
    }

#endif // 0 // ゲージで去っていく処理　デバック中やりにくいから一旦コメントアウト

    switch (state)
    {
    case EBubbleState::QueuingMove:
        position = MathHelper::Lerp(position, targetPos, elapsedTime * moveSpeed);
        SetPosition(position);

        if (MathHelper::Distance(position, targetPos) < 0.05f)
        {
            SetPosition(targetPos);
            state = EBubbleState::Waiting;
        }
        break;

    case EBubbleState::LeavingBack:
        position = MathHelper::Lerp(position, targetPos, elapsedTime * moveSpeed);
        SetPosition(position);

        if (MathHelper::Distance(position, targetPos) < 0.05f)
        {
            // 左に退場
            targetPos = position;
            targetPos.x -= 6.0f;   // 左に消える
            state = EBubbleState::LeavingLeft;
        }
        break;

    case EBubbleState::LeavingLeft:
        position = MathHelper::Lerp(position, targetPos, elapsedTime * moveSpeed);
        SetPosition(position);

        if (MathHelper::Distance(position, targetPos) < 0.05f)
        {
            MarkPendingKill();
        }
        break;
    }

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
    ImGui::Text("Time Limit     : %.1f", timeLimit);
    ImGui::Text("Remaining Time : %.1f", remainingTime);

    ImGui::DragFloat2(U8("UIの吹き出し位置のオフセット"), &uiOffset.x, 0.5f);

#endif
};

// お題を設定する
void OdenBubbleActor::SetOrderAndMakeUi(const OrderData& orderData, const std::string& orderUiFileName)
{
    // ここでお題のデータを入れる
    this->orderData = orderData;

    auto uiManager = GetOwnerScene()->GetUIManager();

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(position);

    // お題のUIコンポーネントを作成する
    std::string filename = "./Data/Textures/UI/" + orderUiFileName + ".png";
    orderUi = std::make_shared<UIImageComponent>(filename, "OdenBubbleUi");
    orderUi->SetWorldPosition({ uiPos.x, uiPos.y });
    orderUi->SetPivot({ 0.5f,0.5f });
    orderUi->SetSize({ 200, 150 });
    uiManager->Add(orderUi);

    // 制限時間を設定する
    timeLimit = MathHelper::RandomRange(4.0f, 6.0f); // 4.0f ~ 6.0f で去る
    remainingTime = timeLimit;

    // 制限時間のゲージのUIを作成する
    gaugeUi = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/boss_hp_frame.png", "./Data/Textures/UI/boss_hp.png", "gauge");
    gaugeUi->SetWorldPosition({ uiPos.x, uiPos.y });
    gaugeUi->SetSize({ 300, 40 });

    uiManager->Add(gaugeUi);

}

// 食材が落とされたら呼ばれる関数
void OdenBubbleActor::OnIngredientDropped(const OdenIngredientActor& ingredient)
{
    if (state != EBubbleState::Waiting) // 状態が待機じゃなかったら
        return;

    // スコアを計算する
    float mathcRate = JudgeMatchShapeRate(ingredient);

    float score = CalculateOrderScore(ingredient, mathcRate, 1.0f); // 今は 1.0fだけど、倍率で変える

    // 状態を去るに変更
    state = EBubbleState::LeavingBack;

    if (onCompleted)
    {// 提出した後に
        onCompleted(*this, score);
    }
    else
    {
        Logger::Warning(U8("提出した時のスコアができていない！"));
    }

    if (scorePopupUi)
    {
        // スコアを表示
        scorePopupUi->Play(L"+" + std::to_wstring(static_cast<int>(score)));
        scorePopupUi->SetVisible(true);
    }
}


float OdenBubbleActor::JudgeMatchShapeRate(const OdenIngredientActor& ingredient) const
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

// 値段と合わせたスコアを計算する
float OdenBubbleActor::CalculateOrderScore(const OdenIngredientActor& ingredient, float matchRate, float multiplier)
{
    float basePrice = ingredient.GetPrice();
    Logger::Log(U8("食材の値段 = ") + std::to_string(basePrice));

    const float rate = matchRate / 100.0f;

    return basePrice * rate * multiplier;
}