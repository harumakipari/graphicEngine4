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

    void Update(float deltaTime)override;

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
    float lifeTimeDuration = 1.0f;
    int popupScore = 0;

    DirectX::XMFLOAT2 uiStartPos={0.0f,0.0f}; // UIの最初の位置
    DirectX::XMFLOAT2 uiTargetPos={0.0f,0.0f}; // UIのターゲットの位置
    float uiTime = 0.0f;
};
