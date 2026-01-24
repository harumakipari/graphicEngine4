#include "pch.h"

#include "OdenUITimerActor.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "UI/FontManager.h"

float EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return  static_cast<float>(1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2));
}
float EaseOutBounce(float t)
{
    if (t < 1 / 2.75f)
        return 7.5625f * t * t;
    else if (t < 2 / 2.75f)
        return 7.5625f * (t -= 1.5f / 2.75f) * t + 0.75f;
    else if (t < 2.5 / 2.75)
        return 7.5625f * (t -= 2.25f / 2.75f) * t + 0.9375f;
    else
        return 7.5625f * (t -= 2.625f / 2.75f) * t + 0.984375f;
}


void OdenUITimerActor::Initialize(const Transform& transform)
{
    const auto scene = GetOwnerScene();

    tensPosition = { 1737.0f, 88.0f };
    onesPosition = { 1835.0f, 88.0f };

    timerOnesUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/number.png", "timer_ones_number");
    timerOnesUi->SetWorldPosition(onesPosition);
    timerOnesUi->SetPivot({ 0.5f,0.5f });
    timerOnesUi->SetSize({ 90, 120 });
    timerOnesUi->zOrder = 100;
    scene->GetUIManager()->Add(timerOnesUi);

    timerTensUi = std::make_shared<UIImageComponent>("./Data/Textures/UI/number.png", "timer_tens_number");
    timerTensUi->SetWorldPosition(tensPosition);
    timerTensUi->SetPivot({ 0.5f,0.5f });
    timerTensUi->SetSize({ 90, 120 });
    timerTensUi->zOrder = 100;
    scene->GetUIManager()->Add(timerTensUi);

    easingRunner = std::make_shared<EasingRunner>();
}

void OdenUITimerActor::Update(float elapsedTime)
{
    easingRunner->Tick(elapsedTime);

    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            remainingTimer = static_cast<int>(gameManager->GetRemainingTime());
        }
    }

    int onesNumber = remainingTimer % 10;
    int tensNumber = remainingTimer / 10;

    timerOnesUi->SetUV({ 150.0f * onesNumber,0.0f,150.0f,200.0f });
    timerTensUi->SetUV({ 150.0f * tensNumber,0.0f,150.0f,200.0f });
    if (tensNumber == 0)
    {
        timerTensUi->SetVisible(false);
    }

    // 秒が変わったら時の処理
    int currentSecond = remainingTimer;
    if (currentSecond != lastSecond)
    {
        lastSecond = currentSecond;
        animTimer = 0.0f;

        if (currentSecond == 10)
        {// 10秒以になったらフェードアウトアニメーションへ
            timerAnimState = ETimerAnimState::FadeOnly;
        }
        else if (currentSecond <= 9)
        {// 9秒以下になったらフェードアウトアニメーションへ
            timerAnimState = ETimerAnimState::FadeOut;

            CoreAudio::PlayOneShot(L"./Data/Sound/SE/game_countDown_se.wav");

            //popupScale = 1.0f;
            //TestEasingHandler handler;
            //handler.AddEasing(TestEaseType::OutElastic, 0.5f, 1.2f, 0.25f);

            //handler.SetCompletedFunction([this]()
            //    {
            //        timerAnimState = ETimerAnimState::FadeOut;
            //        animTimer = 0.0f;
            //    });

            //PropertyAccessor<float> accessor;
            //accessor.getter = [this]() { return popupScale; };
            //accessor.setter = [this](float v) { popupScale = v; };

            //easingRunner->StartHandler(handler, accessor);

        }
        else
        {
            timerAnimState = ETimerAnimState::Normal;
        }
    }


    if (timerAnimState == ETimerAnimState::FadeOnly)
    {// 10秒専用フェードアニメーション
        const float fadeTime = 0.2f;
        animTimer += elapsedTime;

        float t = std::clamp(animTimer / fadeTime, 0.0f, 1.0f);
        float alpha = 1.0f - t;

        timerOnesUi->SetColor({ 1,1,1,alpha });
        timerTensUi->SetColor({ 1,1,1,alpha });

        if (t >= 1.0f)
        {
            // 完全に消えたまま待つ
            timerOnesUi->SetVisible(false);
            timerTensUi->SetVisible(false);
        }

        return;
    }


    if (timerAnimState == ETimerAnimState::FadeOut)
    {// 9秒以下フェードアウトアニメーション
        const float fadeTime = 0.12f;
        animTimer += elapsedTime;

        float t = std::clamp(animTimer / fadeTime, 0.0f, 1.0f);
        float alpha = 1.0f - t;

        timerOnesUi->SetColor({ 1,0,0,alpha });
        timerTensUi->SetColor({ 1,0,0,alpha });

        // 位置を真ん中にする
        timerOnesUi->SetWorldPosition({ (onesPosition.x + tensPosition.x) * 0.5f ,onesPosition.y });

        if (t >= 1.0f)
        {
            timerAnimState = ETimerAnimState::PopWarning;
            animTimer = 0.0f;

            // ポップ準備
            timerOnesUi->SetVisible(true);
            //timerTensUi->SetVisible(true);
            timerOnesUi->SetScale({ 0.5f, 0.5f });
            timerTensUi->SetScale({ 0.5f, 0.5f });
            timerOnesUi->SetColor({ 1,1,1,1 });
            timerTensUi->SetColor({ 1,1,1,1 });
        }

        return;
    }

    if (timerAnimState == ETimerAnimState::PopWarning)
    {
        if (timerAnimState == ETimerAnimState::PopWarning)
        {
            animTimer += elapsedTime;

            // ---- ① ポン！フェーズ ----
            if (popPhase == EPopPhase::Pop)
            {
#if 1
                const float popTime = 0.25f;
                float t = std::clamp(animTimer / popTime, 0.0f, 1.0f);

                float eased = EaseOutBounce(t);   // ← ここが「ぽよん」
                popupScale = std::lerp(0.5f, 1.3f, eased);

                timerOnesUi->SetScale({ popupScale, popupScale });
                timerTensUi->SetScale({ popupScale, popupScale });
                timerOnesUi->SetColor({ 1,1,1,1 });
                timerTensUi->SetColor({ 1,1,1,1 });

                if (t >= 1.0f)
                {
                    // フェードアウトへ
                    popPhase = EPopPhase::FadeOut;
                    animTimer = 0.0f;
                }
                return;
#else

                popPhase = EPopPhase::FadeOut;
#endif // 0

            }

            // ---- ② フェードアウトフェーズ ----
            if (popPhase == EPopPhase::FadeOut)
            {
                const float fadeTime = 0.35f;
                float t = std::clamp(animTimer / fadeTime, 0.0f, 1.0f);

                float alpha = 1.0f - t;

                // 少しだけ余韻でスケールを落とすのも◎
                popupScale = std::lerp(1.3f, 1.8f, t);

                timerOnesUi->SetScale({ popupScale, popupScale });
                timerTensUi->SetScale({ popupScale, popupScale });
                timerOnesUi->SetColor({ 1,1,1,alpha });
                timerTensUi->SetColor({ 1,1,1,alpha });

                if (t >= 1.0f)
                {
                    timerOnesUi->SetVisible(false);
                    timerTensUi->SetVisible(false);
                }
                return;
            }
        }


    }

    //timerOnesUi->SetScale({ popupScale, popupScale });
    //timerTensUi->SetScale({ popupScale, popupScale });

}

