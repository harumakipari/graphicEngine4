#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

// 　
// 　タイマーのUI表示
//
class ScissorsUiTimerActor :public Actor
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
    explicit ScissorsUiTimerActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    // ＋３秒の動き
    void Play();

    // 時計を揺らす
    void StartTimerShake();
private:
    std::shared_ptr<UIImageComponent> timerOnesUi; // タイマーの一桁目のスプライト描画
    std::shared_ptr<UIImageComponent> timerTensUi; // タイマーの十桁目のスプライト描画

    std::shared_ptr<EasingRunner> easingRunner;
    std::shared_ptr<EasingRunner> easingTimerPlus;  // フィーバー時に＋三秒されるやつの動き

    std::shared_ptr<UIImageComponent> timerPlusUi;  // フィーバーによって＋3秒された描画

    int remainingTimer = 0; // 残り時間（秒）

    ETimerAnimState timerAnimState = ETimerAnimState::Normal;
    float animTimer = 0.0f;
    int lastSecond = -1;
    EPopPhase popPhase = EPopPhase::Pop;
    float popupScale = 1.0f;

    XMFLOAT2 tensPosition = {};
    XMFLOAT2 onesPosition = {};
    XMFLOAT2 timerPos = { 1600.0f,200.0f };

    float timerPlusPosition = 0.0f;

    float shakeTimer = 0.0f;
    float shakeDuration = 0.5f;   // 揺れる時間
    float shakeStrength = 8.0f;   // 揺れの強さ（度数）
    bool isShaking = false;
};