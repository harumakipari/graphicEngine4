#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　タイマーのUI表示
//
class OdenUITimerActor :public Actor
{
public:
    enum class ETimerAnimState :uint8_t
    {
        Normal,          // 11秒以上
        FadeOnly,        // 10秒専用（フェードだけ）
        FadeOut,         // ９秒以下（フェードアウト）
        PopWarning         
    };

    enum class EPopPhase
    {
        Pop,        // ぽよん（スケールのみ）
        FadeOut     // フェードアウト
    };

public:
    explicit OdenUITimerActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

private:
    std::shared_ptr<UIImageComponent> timerOnesUi; // タイマーの一桁目のスプライト描画
    std::shared_ptr<UIImageComponent> timerTensUi; // タイマーの十桁目のスプライト描画

    std::shared_ptr<EasingRunner> easingRunner;

    int remainingTimer = 0; // 残り時間（秒）

    ETimerAnimState timerAnimState = ETimerAnimState::Normal;
    float animTimer = 0.0f;
    int lastSecond = -1;
    EPopPhase popPhase = EPopPhase::Pop;
    float popupScale = 1.0f;

    XMFLOAT2 tensPosition={};
    XMFLOAT2 onesPosition = {};
};