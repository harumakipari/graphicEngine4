#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"


// 　
// 　ready go のUI表示
//
class ScissorsUIStartActor :public Actor
{
public:
    explicit ScissorsUIStartActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 演出開始
    void PlayReady(const std::function<void()>& onFinished = nullptr);

    
private:
    void PlayGo();

private:
    std::shared_ptr<UIImageComponent> readyImageComponent; // ready のテキスト
    std::shared_ptr<UIImageComponent> goImageComponent; // go のテキスト
    std::shared_ptr<EasingRunner> easingAlpha;
    std::shared_ptr<EasingRunner> easingPosition;

    std::shared_ptr<EasingRunner> easingGoAlpha;
    std::shared_ptr<EasingRunner> easingGoScale;

    std::function<void()> onStartFinished;  // Goの演出が終わった後の処理を追加

    float easingReadyAlphaValue = 0.0f; // 0.0f~1.0fに変化する値
    float easingReadyPositionValue = 0.0f; // 0.0f~1.0fに変化する値

    float easingGoAlphaValue = 0.0f; // 0.0f~1.0fに変化する値
    float easingGoScaleValue = 0.0f; // 0.0f~1.0fに変化する値


    float fadeInTime = 0.8f;
    float fadeOutTime = 0.3f;

    XMFLOAT2 targetPos = { 0.0f,0.0f };

    float waitInterval = 0.8f; // ready と go の間の待ち時間
};
