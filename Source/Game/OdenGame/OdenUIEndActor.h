#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　finish のUI表示
//
class OdenUIEndActor :public Actor
{
public:
    explicit OdenUIEndActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 演出開始
    void Play();

private:
    std::shared_ptr<UIImageComponent> finishImageComponent; // finish のテキスト
    std::shared_ptr<EasingRunner> easingAlpha;
    std::shared_ptr<EasingRunner> easingScale;

    float easingAlphaValue = 0.0f; // 0.0f~1.0fに変化する値
    float easingScaleValue = 0.0f; // 0.0f~1.0fに変化する値

    float fadeInTime = 0.8f;
    float fadeOutTime = 0.3f;
};
