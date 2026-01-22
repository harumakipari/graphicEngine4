#include "pch.h"
#include "OdenUIFeverGaugeActor.h"

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "Physics/CollisionFunction.h"


void OdenUIFeverGaugeActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
    auto uiManager = GetOwnerScene()->GetUIManager();

    // フィーバーゲージのスプライト描画コンポーネントを追加
    gaugeComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/bar.png", "fever_gauge_ui");
    gaugeComponent->SetWorldPosition({ 67, 965 });
    gaugeComponent->SetScale({ 1.0f, 1.0f });
    gaugeComponent->SetPivot({ 0.0f,0.5f });
    gaugeComponent->SetColor(CoreColor::White);
    uiManager->Add(gaugeComponent);

    // フィーバーゲージのフレームスプライト描画コンポーネントを追加
    gaugeFrameComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/bar_line.png", "bar_frame_ui");
    gaugeFrameComponent->SetWorldPosition({ 67, 965 });
    gaugeFrameComponent->SetScale({ 1.0f, 1.0f });
    gaugeFrameComponent->SetPivot({ 0.0f,0.5f });
    gaugeFrameComponent->SetColor(CoreColor::White);
    uiManager->Add(gaugeFrameComponent);

    // フィーバーゲージのフレームスプライト描画コンポーネントを追加
    gaugeFrameBackComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/bar_back.png", "bar_back_ui");
    gaugeFrameBackComponent->SetWorldPosition({ 67, 965 });
    gaugeFrameBackComponent->SetScale({ 1.0f, 1.0f });
    gaugeFrameBackComponent->SetPivot({ 0.0f,0.5f });
    gaugeFrameBackComponent->SetColor(CoreColor::White);
    uiManager->Add(gaugeFrameBackComponent);


    gaugeUi = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/boss_hp_frame.png", "./Data/Textures/UI/boss_hp.png", "gauge");
    gaugeUi->SetWorldPosition({ 50, 300 });
    gaugeUi->SetSize({ 350, 40 });

    uiManager->Add(gaugeUi);

}

void OdenUIFeverGaugeActor::Update(float elapsedTime)
{
    easingRunner->Tick(elapsedTime);

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager");
    if (!actor) return;

    //　フィーバーをためるコンボを取得する
    auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor);

    int currentCombo = gameManager->GetCombo();

    float feverGauge = gameManager->GetFeverGauge();
    float feverGaugeMax = gameManager->GetFeverGaugeMax();

    if (gaugeUi)
    {
        gaugeUi->SetValue(feverGauge, feverGaugeMax);
        gaugeUi->SetWorldPosition({ uiPos.x, uiPos.y });
    }


#if 0
    if (gaugeComponent)
    {
        gaugeComponent->SetWorldPosition({ uiPos.x, uiPos.y });
        gaugeFrameComponent->SetWorldPosition({ uiPos.x, uiPos.y });
        gaugeFrameBackComponent->SetWorldPosition({ uiPos.x, uiPos.y });
    }
#endif // 0


}

