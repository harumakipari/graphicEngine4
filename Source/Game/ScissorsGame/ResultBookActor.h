#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"
#include "NumberModelDisplay.h"


// リザルト本アクター
class ResultBookActor :public BookBaseActor
{
public:
    explicit ResultBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

protected:
    // コントローラー対応の本が開く処理
    void HandlePadInput() override;

private:
    // 矢印ボタンのUIを作成する
    void CreateButtonArrow();

private:
    DirectX::XMFLOAT3 scoreRelativePosition = { 0.0f,0.0f,0.f };

    NumberDisplay totalScoreDisplay;    // トータルスコア
    NumberDisplay comboDisplay; // コンボ数
    NumberDisplay heartDisplay;    // HPボーナス
    NumberDisplay redirectDisplay;  // 反射ボーナス
    NumberDisplay gatherDisplay;  // まとめボーナス
    NumberDisplay timerDisplay;  // 秒数ボーナス

    NumberDisplay ranking1Display;  // ランキング
    NumberDisplay ranking2Display;  // ランキング
    NumberDisplay ranking3Display;  // ランキング
    NumberDisplay ranking4Display;  // ランキング
    NumberDisplay ranking5Display;  // ランキング


    std::shared_ptr<SkeletalMeshComponent> numberModel;

};

