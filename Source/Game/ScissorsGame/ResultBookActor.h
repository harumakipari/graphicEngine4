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

private:
    DirectX::XMFLOAT3 scoreRelativePosition = { 0.0f,0.0f,0.f };

    NumberDisplay totalScoreDisplay;    // トータルスコア
    NumberDisplay comboDisplay; // コンボ数
    NumberDisplay heartDisplay;    // HPボーナス
    NumberDisplay redirectDisplay;  // 反射ボーナス
    NumberDisplay gatherDisplay;  // まとめボーナス
    NumberDisplay timerDisplay;  // 秒数ボーナス

    std::shared_ptr<SkeletalMeshComponent> numberModel;

};

