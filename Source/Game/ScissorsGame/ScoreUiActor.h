#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　スコアのUI表示
//
class ScoreUiActor :public Actor
{
public:
    explicit ScoreUiActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    // スコアを桁ごとに分解する
    void UpdateScoreDigits(int score) const;


private:
    // スコアの数字描画
    std::vector<std::shared_ptr<UIImageComponent>> scoreDigits;

    std::shared_ptr<UIImageComponent> scoreBackUi;  // スコアの裏
    std::shared_ptr<EasingRunner> easingRunner;

    int prevScore = 0;
    float popupScale = 1.9f;
    XMFLOAT2 scoreBackOffset = { 15.0f,13.0f };
    XMFLOAT2 scoreOffset = { 11.0f,-5.0f };

    DirectX::XMFLOAT2 scorePos = { 1800.0f,95.0f };
};
