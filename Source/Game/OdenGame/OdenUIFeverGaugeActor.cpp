#include "pch.h"
#include "OdenUIFeverGaugeActor.h"

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "Physics/CollisionFunction.h"


DirectX::XMFLOAT4 HSVtoRGB(float h, float s, float v, float a = 1.0f)
{
    h = fmodf(h, 1.0f);
    if (h < 0.0f) h += 1.0f;

    float r = v, g = v, b = v;

    if (s > 0.0f)
    {
        h *= 6.0f;
        int i = static_cast<int>(floorf(h));
        float f = h - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i % 6)
        {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        }
    }

    return DirectX::XMFLOAT4(r, g, b, a);
}

void OdenUIFeverGaugeActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
    auto uiManager = GetOwnerScene()->GetUIManager();

#if 0
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

#endif // 0

    // フィーバーゲージのフレームスプライト描画コンポーネントを追加
    gaugeFrameBackComponent = std::make_shared<UIImageComponent>("./Data/Textures/UI/bar_back.png", "bar_back_ui");
    gaugeFrameBackComponent->SetWorldPosition({ 67, 965 });
    gaugeFrameBackComponent->SetScale({ 1.0f, 1.0f });
    gaugeFrameBackComponent->SetSize({ 400,50 });
    gaugeFrameBackComponent->zOrder = 10;
    //gaugeFrameBackComponent->SetPivot({ 0.0f,0.5f });
    gaugeFrameBackComponent->SetColor(CoreColor::White);
    uiManager->Add(gaugeFrameBackComponent);


    gaugeUi = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/bar_line_yellow.png", "./Data/Textures/UI/bar.png", "gauge");
    gaugeUi->SetWorldPosition({ 50, 300 });
    gaugeUi->zOrder = 15;
    gaugeUi->SetSize({ 400,50 });

    uiManager->Add(gaugeUi);

}

void OdenUIFeverGaugeActor::Update(float elapsedTime)
{
    totalTime += elapsedTime;

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


    float feverGauge = gameManager->GetFeverGauge();
    float feverGaugeMax = gameManager->GetFeverGaugeMax();


    if (feverGauge > 0.0f && !gameManager->IsFeverMode())
    {// 
        gaugeUi->SetColor({ 1.0f, 1.0f, 0.3f, 1.0f }); // 黄色
        gaugeUi->SetGaugeFrameColor(CoreColor::White);
    }
    else
    {
        gaugeUi->SetColor(CoreColor::White);
        gaugeUi->SetGaugeFrameColor({0.8f,0.8f,0.8f,1.0});
    }

    if (gameManager->IsFeverMode())
    {
        static XMFLOAT4 colors[] =
        {
            {1.0f, 0.3f, 0.8f, 1.0f},
            {1.0f, 1.0f, 0.2f, 1.0f},
            {0.3f, 1.0f, 0.5f, 1.0f},
        };

        int idx = static_cast<int>(totalTime * 8.0f) % 3;
        gaugeUi->SetColor({ colors[idx].x,colors[idx].y,colors[idx].z,colors[idx].w });
    }

    //float hue = fmod(totalTime * 0.2f, 1.0f); // ゆっくり回す
    //XMFLOAT4 color = HSVtoRGB(hue, 1.0f, 1.0f);

    if (gaugeUi)
    {
        gaugeUi->SetValue(feverGauge, feverGaugeMax);
        gaugeUi->SetWorldPosition({ uiPos.x, uiPos.y });
        //gaugeUi->SetColor({ color.x,color.y,color.z,color.w });
        gaugeUi->SetGaugeOffset(offset);

        gaugeFrameBackComponent->SetWorldPosition({ uiPos.x, uiPos.y });
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

void OdenUIFeverGaugeActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat2("gaugeOffset", &offset.x, 0.5f);
#endif
}
