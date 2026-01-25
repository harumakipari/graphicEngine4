#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　フィーバーゲージのUI表示
//
class OdenUIFeverGaugeActor :public Actor
{
private:
    struct FeverChar
    {
        std::shared_ptr<UIImageComponent> image;
        std::shared_ptr<EasingRunner> scaleRunner;
        std::shared_ptr<EasingRunner> alphaRunner;

        float scale = 1.0f;
        float alpha = 0.0f;
        float hueOffset = 0.0f;
    };
public:
    explicit OdenUIFeverGaugeActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    // FEVER時に再生する
    void PlayFever();
private:
    std::shared_ptr<UIImageComponent> gaugeFrameBackComponent;  // フィーバーゲージのスプライト描画

    std::shared_ptr<UIGaugeComponent> gaugeUi; // 残り時間のゲージUI

    float totalTime = 0.0f;
    XMFLOAT2 offset = { 11.5f,0.0f };

    std::vector<FeverChar> feverChars;

    bool performFeverWord = false;  // Feverの文字演出
};
