#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　フィーバーゲージのUI表示
//
class OdenUIFeverGaugeActor :public Actor
{
public:
    explicit OdenUIFeverGaugeActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    std::shared_ptr<UIImageComponent> gaugeComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<UIImageComponent> gaugeFrameComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<UIImageComponent> gaugeFrameBackComponent;  // フィーバーゲージのスプライト描画
    std::shared_ptr<EasingRunner> easingRunner;

    std::shared_ptr<UIGaugeComponent> gaugeUi; // 残り時間のゲージUI

    float totalTime = 0.0f;
    XMFLOAT2 offset = { 11.5f,0.0f };
};
