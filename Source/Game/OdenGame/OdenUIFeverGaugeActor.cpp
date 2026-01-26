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
    auto uiManager = GetOwnerScene()->GetUIManager();

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


    XMFLOAT2 wordSize = { 400.0f,410.0f };
    XMFLOAT2 wordPos = { 720.0f,450.0f };

    auto createChar = [&](const char* texPath, XMFLOAT2 pos)
        {
            FeverChar c;
            c.image = std::make_shared<UIImageComponent>(texPath, texPath);
            c.image->SetWorldPosition(pos);
            c.image->SetPivot({ 0.5f, 0.5f });
            c.image->SetSize(wordSize);
            c.image->SetColor(XMFLOAT4{ 1,1,1,0 });
            uiManager->Add(c.image);

            c.scaleRunner = std::make_shared<EasingRunner>();
            c.alphaRunner = std::make_shared<EasingRunner>();
            return c;
        };


    feverChars.push_back(createChar("./Data/Textures/UI/word_F.png", wordPos));
    wordPos.x += 180.0f;
    feverChars.push_back(createChar("./Data/Textures/UI/word_E.png", wordPos));
    wordPos.x += 180.0f;
    feverChars.push_back(createChar("./Data/Textures/UI/word_V.png", wordPos));
    wordPos.x += 180.0f;
    feverChars.push_back(createChar("./Data/Textures/UI/word_E.png", wordPos));
    wordPos.x += 180.0f;
    feverChars.push_back(createChar("./Data/Textures/UI/word_R.png", wordPos));

    for (int i = 0; i < feverChars.size(); ++i)
    {
        feverChars[i].hueOffset = i * 0.12f;
    }
}

void OdenUIFeverGaugeActor::Update(float elapsedTime)
{
    totalTime += elapsedTime;

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
        gaugeUi->SetColor(XMFLOAT4{ 1.0f, 1.0f, 0.3f, 1.0f }); // 黄色
        gaugeUi->SetGaugeFrameColor(CoreColor::White);
    }
    else
    {
        gaugeUi->SetColor(CoreColor::White);
        gaugeUi->SetGaugeFrameColor({ 0.8f,0.8f,0.8f,1.0 });
    }

    bool isFever = gameManager->IsFeverMode();

    if (isFever)
    {
        static XMFLOAT4 colors[] =
        {
            {1.0f, 0.3f, 0.8f, 1.0f},
            {1.0f, 1.0f, 0.2f, 1.0f},
            {0.3f, 1.0f, 0.5f, 1.0f},
        };

        int idx = static_cast<int>(totalTime * 8.0f) % 3;
        gaugeUi->SetColor(XMFLOAT4{ colors[idx].x,colors[idx].y,colors[idx].z,colors[idx].w });
    }

    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            if (gameManager->ConsumeFeverWordAppear())
                PlayFever();
        }
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
    float baseHue = fmod(totalTime * 0.2f, 1.0f);


    for (auto& c : feverChars)
    {
        c.scaleRunner->Tick(elapsedTime);
        c.alphaRunner->Tick(elapsedTime);

        float hue = fmod(baseHue + c.hueOffset, 1.0f);

        XMFLOAT4 col = HSVtoRGB(hue, 1.0f, 1.0f, c.alpha);
        c.image->SetColor(col);
        c.image->SetScale({ c.scale, c.scale });
    }
}

void OdenUIFeverGaugeActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat2("gaugeOffset", &offset.x, 0.5f);
    if (ImGui::Button("PlayFever"))
    {
        PlayFever();
    }
#endif
}

// FEVER時に再生する
void OdenUIFeverGaugeActor::PlayFever()
{
    const float charDelay = 0.12f;
    const float fadeInTime = 0.2f;
    const float waitTime = 1.0f;
    const float fadeOutTime = 0.3f;

    for (int i = 0; i < feverChars.size(); ++i)
    {
        auto& c = feverChars[i];

        TestEasingHandler handler;

        handler.AddWait(i * charDelay);

        handler.AddEasing(TestEaseType::OutExp, 0.0f, 1.3f, fadeInTime);
        handler.AddEasing(TestEaseType::OutQuad, 1.3f, 1.0f, waitTime);

        PropertyAccessor<float> scaleAccessor;
        scaleAccessor.getter = [&c]() { return c.scale; };
        scaleAccessor.setter = [&c](float v) { c.scale = v; };

        c.scaleRunner->StartHandler(handler, scaleAccessor);
    }

    for (int i = 0; i < feverChars.size(); ++i)
    {
        auto& c = feverChars[i];

        TestEasingHandler handler;

        handler.AddWait(i * charDelay);
        handler.AddEasing(TestEaseType::InSine, 0.0f, 1.0f, fadeInTime);

        PropertyAccessor<float> alphaAccessor;
        alphaAccessor.getter = [&c]() { return c.alpha; };
        alphaAccessor.setter = [&c](float v) { c.alpha = v; };

        c.alphaRunner->StartHandler(handler, alphaAccessor);
    }


    const float totalDelay =
        charDelay * (feverChars.size() - 1) + fadeInTime + waitTime;

    for (auto& c : feverChars)
    {
        TestEasingHandler handler;

        handler.AddWait(totalDelay);
        handler.AddEasing(TestEaseType::OutExp, 1.0f, 0.0f, fadeOutTime);

        PropertyAccessor<float> alphaAccessor;
        alphaAccessor.getter = [&c]() { return c.alpha; };
        alphaAccessor.setter = [&c](float v) { c.alpha = v; };

        c.alphaRunner->StartHandler(handler, alphaAccessor);
    }

}