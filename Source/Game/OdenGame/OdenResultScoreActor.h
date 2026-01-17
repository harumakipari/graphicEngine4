#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　リザルトスコアのUI表示
//
class OdenResultScoreActor :public Actor
{
public:
    explicit OdenResultScoreActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // フォントをセットする
    void SetFontAndMakeTextComponent();

private:
    std::shared_ptr<UITextComponent> scoreTextUi; // スコアのテキスト描画

};
