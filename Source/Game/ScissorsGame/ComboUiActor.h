#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

// 　
// 　コンボのUI表示
//
class ComboUiActor :public Actor
{
public:
    explicit ComboUiActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    // スコアを桁ごとに分解する
    void UpdateScoreDigits(int combo) const;


private:
    // スコアの数字描画
    std::vector<std::shared_ptr<UIImageComponent>> comboDigits;
    std::shared_ptr<EasingRunner> easingRunner;
    std::shared_ptr<UIImageComponent> scoreBackUi;  // スコアの裏
};
