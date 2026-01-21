#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　スコアのUI表示
//
class OdenUIScoreViewActor :public Actor
{
public:
    explicit OdenUIScoreViewActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override{}

    // フォントをセットする
    void SetFontAndMakeTextComponent();

private:
    std::shared_ptr<UITextComponent> scoreTextUi; // スコアのテキスト描画

    int prevScore = 0;
    float popupTimer = 0.0f;
};
