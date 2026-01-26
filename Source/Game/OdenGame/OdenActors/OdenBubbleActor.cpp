#include "pch.h"
#include "OdenBubbleActor.h"

#include <magic_enum.hpp>

#include "StarParticleActor.h"
#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenGameSession.h"
#include "Game/OdenGame/SubmitSEPlayer.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenActors/OdenIngredientActor.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"
#include "Game/OdenGame/OdenManagers/OdenGameManager.h"

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
    scorePopupUi->zOrder = 10;
    GetOwnerScene()->GetUIManager()->Add(scorePopupUi);

#endif // 0
    state = EBubbleState::Waiting;

    // 星のコンポーネントを追加
    starSpawnParticleComponent = this->AddComponent<class ParticleComponent>("starSpawnParticleComponent", parentName);
    starSpawnParticleComponent->Load("./Data/Effect/Files/starEffect.json");
    starSpawnParticleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    //// ループ再生設定
    //ParticleComponent::AddSettings settings
    //{
    //    .loop = true, // ループ再生
    //};
    //starSpawnParticleComponent->SetAddSettings(settings);

    // 星のコンポーネントを追加
    starAttractParticleComponent = this->AddComponent<class ParticleComponent>("starAttractParticleComponents", parentName);
    starAttractParticleComponent->Load("./Data/Effect/Files/starAttractEffect.json");
    starAttractParticleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
    };
    starAttractParticleComponent->SetAddSettings(settings);

    // イージング
    easingRunner = std::make_shared<EasingRunner>();

    // 失敗のエフェクトコンポーネントを追加
    failParticleComponent = this->AddComponent<class ParticleComponent>("failParticleComponent", parentName);
    failParticleComponent->Load("./Data/Effect/Files/failEffect.json");
    failParticleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
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
    bool canInput = true;

    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            canInput = gameManager->IsGameInputEnabled();
        }
    }


    easingRunner->Tick(elapsedTime);

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    if (orderUi)
        orderUi->SetWorldPosition({ uiPos.x, uiPos.y });



    if (scorePopupUi)
    {
        XMFLOAT2 scoreUiPos = { uiPos.x + scorePopUiOffset.x,uiPos.y + scorePopUiOffset.y };
        scorePopupUi->SetWorldPosition(scoreUiPos);
    }

    XMFLOAT2 gaugeUiPos = { uiPos.x + uiOffset.x,uiPos.y + uiOffset.y };

    if (gaugeUi)
    {
        gaugeUi->SetValue(remainingTime, timeLimit);
        gaugeUi->SetWorldPosition({ gaugeUiPos.x, gaugeUiPos.y });
    }

#if 1
    // ゲージで去っていく処理　デバック中やりにくいから一旦コメントアウト
    // 待機
    if (canInput)
    {// 演出中じゃないときに
        if (state == EBubbleState::Waiting && canAcceptOrder)
        {
            // 残り時間を減らす
            remainingTime -= elapsedTime;

            if (remainingTime <= 0.0f)
            {
                remainingTime = 0.0f;
                state = EBubbleState::LeavingBack;

                Logger::Log(U8("時間切れで自動失敗"));

                // スコア 0 で通知
                if (onCompleted)
                    onCompleted(*this, { 0.0f,0.0f });
            }
        }

        // 焦り度
        float timeRate = remainingTime / timeLimit; // 1.0->0.0
        if (orderUi && timeRate < 0.3f && state == EBubbleState::Waiting)
        {// 揺らす
            shakeTimer += elapsedTime;

            float panic = (0.4f - timeRate) / 0.4f; // 0~1.0
            panic = std::clamp(panic, 0.0f, 1.0f);

            float shakePower = 6.0f * panic;      // 揺れ幅
            float shakeSpeed = 25.0f + 40.0f * panic;

            float shakeX = sinf(shakeTimer * shakeSpeed) * shakePower;

            orderUi->SetWorldPosition({
                uiPos.x + shakeX,
                uiPos.y
                });

            float rot = sinf(shakeTimer * 35.0f) * 8.0f * panic;
            orderUi->SetWorldAngleDegree(rot);
        }
        else
        {
            shakeTimer = 0.0f;
            orderUi->SetWorldAngleDegree(0.0f);
        }
    }


#endif // 0 // ゲージで去っていく処理　デバック中やりにくいから一旦コメントアウト


    switch (state)
    {
    case EBubbleState::Entering:
    {
        if (MoveTowards(targetPos, moveSpeed, elapsedTime))
        {
            state = EBubbleState::Waiting;
        }
        break;
    }
    case EBubbleState::LeavingBack:
        if (MoveTowards(targetPos, moveSpeed, elapsedTime))
        {
            // 次は「斜め後ろの共通退場地点」へ
            targetPos = { -14.0f, 0.0f, 23.0f };
            state = EBubbleState::LeavingLeft;
        }
        break;

    case EBubbleState::LeavingLeft:
        if (MoveTowards(targetPos, moveSpeed, elapsedTime))
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
    ImGui::DragFloat2(U8("スコアの数字のオフセット"), &scorePopUiOffset.x, 0.5f);

#endif
};

// お題を設定する
void OdenBubbleActor::SetOrderAndMakeUi(const OrderData& orderData, const std::string& orderUiFileName, bool useTimeLimit)
{
    // ここでお題のデータを入れる
    this->orderData = orderData;

    auto uiManager = GetOwnerScene()->GetUIManager();

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(position);

    // お題のUIコンポーネントを作成する
    std::string filename = "./Data/Textures/UI/UI_Order/" + orderUiFileName + ".png";
    orderUi = std::make_shared<UIImageComponent>(filename, "OdenBubbleUi");
    orderUi->SetWorldPosition({ uiPos.x, uiPos.y });
    orderUi->SetPivot({ 0.5f,0.5f });
    orderUi->SetSize({ 200, 150 });
    uiManager->Add(orderUi);
    if (useTimeLimit)
    {
        // 制限時間を設定する
        timeLimit = MathHelper::RandomRange(10.0f, 15.0f); // 
        remainingTime = timeLimit;
    }
    else
    {
        timeLimit = 1000.0f; // 事実上無制限
        remainingTime = timeLimit;
    }
    // 制限時間のゲージのUIを作成する
#if 0
    gaugeUi = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/boss_hp_frame.png", "./Data/Textures/UI/boss_hp.png", "gauge");
    gaugeUi->SetWorldPosition({ uiPos.x, uiPos.y });
    gaugeUi->SetSize({ 300, 40 });

    uiManager->Add(gaugeUi);

#endif // 0
}

// 食材が落とされたら呼ばれる関数
void OdenBubbleActor::OnIngredientDropped(const OdenIngredientActor& ingredient)
{
    if (!CanAcceptIngredient())
        return;

    isCompeted = true;
    canAcceptOrder = false;

    // スコアを計算する
    float matchRate = JudgeMatchShapeRate(ingredient);
    EScore score = JudgeScoreFromRate(matchRate);

    float sales = CalculateSales(ingredient, score);
    float satisfaction = GetSatisfactionValue(score);

    // 状態を去るに変更
    state = EBubbleState::LeavingBack;



    if (onCompleted)
    {// 提出した後に
        onCompleted(*this, { sales,satisfaction });
    }
    else
    {
        Logger::Warning(U8("提出した時のスコアができていない！"));
    }

    // 表示系
    if (scorePopupUi)
    {
        if (score == EScore::Perfect)
        {
            // 提出音　成功SE再生

            SubmitSEPlayer::Play();
            //CoreAudio::PlayOneShot(L"./Data/Sound/SE/succeed_submit.wav");
            // 総合スコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    bool wasFever = gameManager->IsFeverMode();
                    OdenGameSession::Instance().submitLogs.emplace_back(submittedIngredientType, 1, sales, wasFever);
                    float scoreValue = wasFever ? 2.0f * sales : sales;
                    scorePopupUi->Play(std::to_wstring(static_cast<int>(scoreValue)));
                }
            }

        }
        else if (score == EScore::Good)
        {
            // 提出音　成功SE再生
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/succeed_submit.wav");
            // 総合スコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    bool wasFever = gameManager->IsFeverMode();
                    OdenGameSession::Instance().submitLogs.emplace_back(submittedIngredientType, 1, sales, wasFever);
                    float scoreValue = wasFever ? 2.0f * sales : sales;
                    scorePopupUi->Play(std::to_wstring(static_cast<int>(scoreValue)));
                }
            }
        }
        else
        {
            scorePopupUi->Play(std::to_wstring(0));
            // 提出音　失敗SE再生
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/fail_submit.wav");

            if (failParticleComponent)
            {// 失敗のエフェクトを出す
                failParticleComponent->Play();
            }

            // 総合スコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    // コンボをなくす
                    gameManager->OnSubmitMiss();
                }
            }

        }

        scorePopupUi->SetVisible(true);
    }

    // エフェクトを出す
    if (starSpawnParticleComponent)
    {
        if (score == EScore::Perfect)
        {
            starSpawnParticleComponent->Play();

            Transform starTransform;
            starTransform.SetTranslation(GetPosition());
            auto starActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<StarParticleActor>("starParticle", starTransform);
            // 総合スコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    // コンボを加算する フィーバーゲージが溜まる
                    gameManager->OnSubmitSuccess();
                    bool wasFever = gameManager->IsFeverMode();
                    float scoreValue = wasFever ? 2.0f * sales : sales;

                    starActor->StartParticle(static_cast<int>(scoreValue));
                }
            }
        }
        else if (score == EScore::Good)
        {
            starSpawnParticleComponent->Play();

            Transform starTransform;
            starTransform.SetTranslation(GetPosition());
            auto starActor = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<StarParticleActor>("starParticle", starTransform);
            // 総合スコアを加算する
            if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
            {
                if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
                {
                    // コンボを加算する フィーバーゲージが溜まる
                    gameManager->OnSubmitSuccess();
                    bool wasFever = gameManager->IsFeverMode();
                    float scoreValue = wasFever ? 2.0f * sales : sales;

                    starActor->StartParticle(static_cast<int>(scoreValue));
                }
            }
        }
        else
        {
            //particleComponent->Play();
        }
    }

    isCompeted = true;
}

// 毎フレーム呼ばれる (0 ~ 1)
void OdenBubbleActor::OnBeatPhase(float phase)
{
    float pulse = sinf(phase * DirectX::XM_2PI);
    float scale = 1.0f + pulse * 0.001f;

    if (orderUi)
        orderUi->SetScale({ scale, scale });
}


EScore OdenBubbleActor::JudgeScoreFromRate(const float matchRate) const
{
    if (matchRate >= 100.0f)
        return EScore::Perfect;

    if (matchRate <= 0.0f)
        return EScore::Fail;

    return EScore::Good;
}

float OdenBubbleActor::JudgeMatchShapeRate(const OdenIngredientActor& ingredient)
{
    const OrderData& o = orderData;

    // 提出された食材の種類を入れる
    //submittedIngredientType = o.requiredIngredient;
    submittedIngredientType = ingredient.GetIngredientType();

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

float OdenBubbleActor::CalculateSales(const OdenIngredientActor& ingredient, const EScore score)
{
    if (score == EScore::Fail)
        return 0.0f;

    if (score == EScore::Good)
        return 50.0f;

    return ingredient.GetPrice();
}

// 満足度の計算
float OdenBubbleActor::GetSatisfactionValue(EScore score) const
{
    switch (score)
    {
    case EScore::Perfect: return 1.0f; // 大きく増える
    case EScore::Good:    return 0.4f; // 少し増える
    case EScore::Fail:    return 0.0f; // 増えない or 減らす
    }
    return 0.0f;
}

float OdenBubbleActor::JudgeShapeScore(const OdenShapeData& shape) const
{
    if (NormalizeShapeCategory(shape.category) != NormalizeShapeCategory(orderData.requiredCategory))
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

// ターゲットへ同じ速度で移動する関数
bool OdenBubbleActor::MoveTowards(const XMFLOAT3& target, float speed, float deltaTime)
{
    using namespace DirectX;
    XMFLOAT3 position = GetPosition();
    XMVECTOR p = XMLoadFloat3(&position);
    XMVECTOR t = XMLoadFloat3(&target);

    XMVECTOR dir = t - p;
    float dist = XMVectorGetX(XMVector3Length(dir));

    if (dist < 0.01f)
    {
        SetPosition(target);
        return true; // 到着
    }

    XMVECTOR dirN = XMVector3Normalize(dir);
    float move = speed * deltaTime;

    if (move >= dist)
    {
        SetPosition(target);
        return true;
    }

    p += dirN * move;
    XMFLOAT3 newPos;
    XMStoreFloat3(&newPos, p);
    SetPosition(newPos);

    return false;
}