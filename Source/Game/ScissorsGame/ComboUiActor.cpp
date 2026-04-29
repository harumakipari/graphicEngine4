#include "pch.h"

#include "ComboUiActor.h"

#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void ComboUiActor::Initialize(const Transform& transform)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    for (int i = 0; i < 2; i++) // 2桁くらい確保
    {
        auto digit = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/number.png", "ScoreDigit");

        digit->SetSize({ 90, 120 });
        digit->SetPivot({ 0.5f, 0.5f });

        uiManager->Add(digit);
        comboDigits.push_back(digit);
    }

#if 0
    stamp = std::make_shared<UIImageComponent>(
        "./Data/Textures/ScissorsUI/1.png",
        "ComboStamp"
    );
    stamp->SetSize({ 90, 120 });
    stamp->SetPivot({ 0.5f, 0.5f });
    stamp->SetWorldPosition({ 200,200 });
    uiManager->Add(stamp);
#endif // 0

    stampStructs.resize(10);

    for (int i = 0; i < 10; i++)
    {
        std::string textureName = "./Data/Textures/ScissorsUI/" + std::to_string(i) + ".png";
        std::string componentName = "ComboStamp_" + std::to_string(i);
        stampStructs[i].comboNumberUi = std::make_shared<UIImageComponent>(
            textureName,
            componentName
        );
        stampStructs[i].comboNumberUi->SetSize({ 90, 120 });
        stampStructs[i].comboNumberUi->SetPivot({ 0.5f, 0.5f });
        stampStructs[i].comboNumberUi->SetWorldPosition({ 200,200 });
        uiManager->Add(stampStructs[i].comboNumberUi);

        stampStructs[i].easingRunner = std::make_shared<EasingRunner>();
    }
}

void ComboUiActor::Update(float elapsedTime)
{
    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (!player)
    {
        Logger::Error(U8("ComboUiActorでplayerがnullptrです。"));
        return;
    }

    auto& scoreSystem = player->scoreSystem;

    currentCombo = scoreSystem.GetCombo();
    UpdateScoreDigits(currentCombo);

    float digitSpacing = 90.0f; // 桁の間隔（調整ポイント）

    for (int i = 0; i < comboDigits.size(); i++)
    {
        DirectX::XMFLOAT2 pos = uiPos;

        // i=0が1の位 → 右端
        // iが増えるほど左へ
        pos.x -= i * digitSpacing;

        comboDigits[i]->SetWorldPosition(pos);
    }


    for (auto stamp : stampStructs)
    {
        if (stamp.comboNumberUi)
        {
            stamp.comboNumberUi->SetScale({ stamp.stampScale, stamp.stampScale });
            stamp.comboNumberUi->SetVisible(stamp.isVisible);
        }
        stamp.easingRunner->Tick(elapsedTime);
    }

#if 0
    if (currentCombo > prevCombo)
    {// コンボが増えたら
        AddCombo(currentCombo);
        Logger::Log(U8("コンボが増えた:") + std::to_string(currentCombo));
    }

#endif // 0

    if (currentCombo == 0 && prevCombo != 0)
    {
        Logger::Log(U8("コンボがリセットされた"));

        for (int i = 1; i < 10; i++)
        {
            stampStructs[i].isVisible = false;
            stampStructs[i].stampScale = 10.0f; // 初期値に戻す

            if (stampStructs[i].comboNumberUi)
            {
                stampStructs[i].comboNumberUi->SetVisible(false);
            }
        }
    }

    prevCombo = currentCombo;// コンボの値を保存する
}

void ComboUiActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    static     int a = 0;
    if (ImGui::Button(U8("コンボスタンプ")))
    {
        AddCombo(a);
    }
    ImGui::DragInt("Combo", &a);
#endif
}


// スコアを桁ごとに分解する
void ComboUiActor::UpdateScoreDigits(int combo)
{
#if 1
    // 全部非表示
    for (auto& d : comboDigits)
    {
        d->SetVisible(false);
    }

    int i = 0;

    do
    {
        int digit = combo % 10;

        comboDigits[i]->SetUV({ 150.0f * digit, 0.0f, 150.0f, 200.0f });
        comboDigits[i]->SetVisible(true);

        combo /= 10;
        i++;

    } while (combo > 0 && i < comboDigits.size());
#endif // 0

}

// コンボが足される時の表現
void ComboUiActor::AddCombo(int combo)
{
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/stamp_budge.wav", 2.0f);

    int index = combo;


    if (10 < combo)
    {
        return;
    }

#if 0
    float fadeInTime = 0.8f;

    // フェードイン のスケールを触る
    {
        TestEasingHandler handler;
        handler.AddWait(0.1f);

        handler.AddEasing(
            TestEaseType::OutExp,
            10.0f,
            1.0f,
            fadeInTime
        );

        handler.SetCompletedFunction([this]()
            {
                stamp->SetScale({ 1.0f,1.0f });
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return stampScale; };
        accessor.setter = [this](float t)
            {
                stampScale = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }
#else
    float fadeInTime = 0.3f;


    // フェードイン のスケールを触る
    {
        stampStructs[index].isVisible = true;

        TestEasingHandler handler; // UIのハンドラー
        handler.AddWait(0.1f);

        handler.AddEasing(
            TestEaseType::OutExp,
            10.0f,
            1.0f,
            fadeInTime
        );

        handler.SetCompletedFunction([this, index]()
            {
                stampStructs[index].comboNumberUi->SetScale({ 1.0f,1.0f });
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this, index]() { return stampStructs[index].stampScale; };
        accessor.setter = [this, index](float t)
            {
                stampStructs[index].stampScale = t;
            };

        stampStructs[index].easingRunner->StartHandler(handler, accessor);
    }

#endif // 0


}