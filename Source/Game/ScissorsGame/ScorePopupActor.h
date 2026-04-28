#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　スコアのポップアップUI表示
//
class ScorePopupActor :public Actor
{
public:
    explicit ScorePopupActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // スコアを設定する
    void SetScore(int score);
private:
    // スコアを桁ごとに分解する
    void UpdateScoreDigits(int score) const;

private:
    // スコアの数字描画
    std::vector<std::shared_ptr<UIImageComponent>> scoreDigits;

    std::shared_ptr<EasingRunner> easingRunner;

    float lifeTime = 1.0f;
    float lifeTimeDuration = 3.0f;
    int popupScore = 0;
};
