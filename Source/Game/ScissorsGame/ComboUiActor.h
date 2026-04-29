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
    void UpdateScoreDigits(int combo) ;

    // コンボが足される時の表現
    void AddCombo(int currentCombo);
private:
    // スコアの数字描画
    std::vector<std::shared_ptr<UIImageComponent>> comboDigits;
    std::shared_ptr<UIImageComponent> scoreBackUi;  // スコアの裏

    int prevCombo = 0; // 前回のコンボ値
    int currentCombo = 0; // 現在のコンボ値

    struct Stamp
    {
        std::shared_ptr<EasingRunner> easingRunner;
        std::shared_ptr<UIImageComponent> comboNumberUi;  // 数字
        bool isVisible = false;
        float stampScale = 10.0f;
    };
    std::vector<Stamp> stampStructs;

};
