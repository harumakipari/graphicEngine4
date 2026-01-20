#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"
#include "UI/Widgets/Widget.h"


enum class EScore :uint8_t
{
    Perfect,
    Good,
    Fail,
};

struct OdenResult
{
    float price = 0.0f;
    float satisfaction = 0.0f;
};

class OdenIngredientActor;

// 　お題
// 　ふきだし
//
class OdenBubbleActor :public Actor, public IBeatReactive
{
public:
    // お題の状態
    enum class EBubbleState :uint8_t
    {
        Entering,    // 入ってくる
        Waiting, // 待っている
        LeavingBack,   // 少し後ろに下がる
        LeavingLeft,    // 左に退場
        Completed,    // 注文完了して去った
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

    // 注文を完了して離れる
    void SetLeaving()
    {
        // 少し後ろに下がる
        targetPos = GetPosition();
        targetPos.z += 2.0f;   // 奥方向  ここでターゲットを設定
        state = EBubbleState::LeavingBack;
    }

    // 注文できるかどうか
    bool CanAcceptIngredient() const
    {
        return canAcceptOrder && state == EBubbleState::Waiting && !isCompeted;
    }

    // 拍が来た瞬間の処理
    void OnBeat(bool isStrong) override {}

    // 毎フレーム呼ばれる (0 ~ 1)
    void OnBeatPhase(float phase) override;

    // 実際に提出された食材の種類を取得する
    EOdenType GetIngredientType()const
    {
        return submittedIngredientType;
    }

    void SetTargetPosition(const DirectX::XMFLOAT3& pos)
    {
        targetPos = pos;
        state = EBubbleState::Entering;
    }
private:
    // マッチ率をスコアに変換する
    EScore JudgeScoreFromRate(float matchRate) const;

    // 判定を判定する
    float JudgeMatchShapeRate(const OdenIngredientActor& ingredient);

    // 元々の売り上げ
    float CalculateSales(const OdenIngredientActor& ingredient, EScore score);

    // 満足度の計算
    float GetSatisfactionValue(EScore score) const;

    // 形からスコアを判定する
    float JudgeShapeScore(const OdenShapeData& shape) const;

    // ターゲットへ同じ速度で移動する関数
    bool MoveTowards(const XMFLOAT3& target, float speed, float deltaTime);

public:
    std::function<void(OdenBubbleActor&, OdenResult score)> onCompleted;     // コールバック関数

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

    DirectX::XMFLOAT3 targetPos;
    float moveSpeed = 4.0f;

    bool canAcceptOrder = true; // 注文ができるかどうか

    DirectX::XMFLOAT3 leaveFinalPos; // 最終的に消える位置

    EOdenType submittedIngredientType = EOdenType::None;// 実際に提出された食材の種類

    std::shared_ptr<ParticleComponent> starSpawnParticleComponent;   // 星の発生エフェクト
    std::shared_ptr<ParticleComponent> starAttractParticleComponent;   // 星のスコアへ吸い込まれるエフェクト

    float shakeTimer = 0.0f;    // シェイクタイマー

    bool isCompeted = false;    // 提出完了したかどうか
};
