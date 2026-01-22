#include "pch.h"

#include "OdenUIScoreViewActor.h"

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenGameManager.h"
#include "Physics/CollisionFunction.h"
#include "UI/FontManager.h"

void OdenUIScoreViewActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_shared<EasingRunner>();
}

void OdenUIScoreViewActor::Update(float elapsedTime)
{
    easingRunner->Tick(elapsedTime);

    // UIの位置
    DirectX::XMFLOAT3 position = GetPosition();
    DirectX::XMFLOAT3 bubbleWorldPos = { position.x , position.y, position.z };
    // ワールド座標からUI座標系に変換する
    XMFLOAT2 uiPos = WorldToUI(bubbleWorldPos);

    auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager");
    if (!actor) return;

    // 総合スコアを加算する
    auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor);
    int currentScore = static_cast<int>(gameManager->GetTotalScore());

    if (scoreTextUi)
    {
        scoreTextUi->SetWorldPosition({ uiPos.x, uiPos.y });
        scoreTextUi->SetText(std::to_wstring(currentScore));
        scoreTextUi->SetScale({ popupScale,popupScale });
    }

    if (currentScore > prevScore)
    {
        popupScale = 1.0f;

        TestEasingHandler handler;
        handler.AddEasing(TestEaseType::OutElastic, 0.5f, 1.2f, 0.8f);

        handler.SetCompletedFunction([this]()
            {
            });

        PropertyAccessor<float> accessor;
        accessor.getter = [this]() { return popupScale; };
        accessor.setter = [this](float t)
            {
                popupScale = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }

    prevScore = currentScore;

}

// フォントをセットする
void OdenUIScoreViewActor::SetFontAndMakeTextComponent()
{
    // スコアテキストのUIコンポーネントを作成する
    scoreTextUi = std::make_shared<UITextComponent>("scoreFont");
    scoreTextUi->SetWorldPosition({ 67, 965 });
    scoreTextUi->SetScale({ 1.0f, 1.0f });
    scoreTextUi->SetPivot({ 0.5f,0.5f });
    scoreTextUi->SetColor(CoreColor::White);
    auto uiManager = GetOwnerScene()->GetUIManager();
    uiManager->Add(scoreTextUi);
}
