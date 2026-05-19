#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"
#include "NumberModelDisplay.h"

// タイトル本アクター
class TitleBookActor :public BookBaseActor
{
public:
    explicit TitleBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

protected:
    // コントローラー対応の本が開く処理
    void HandlePadInput() override;


private:
    // UIの矢印ボタンを生成する
    void CreateButtonArrow();

private:
    NumberDisplay firstStageHighScoreDisplay;   
    NumberDisplay bobbinStageHighScoreDisplay;   
    NumberDisplay redirectStageHighScoreDisplay;   
    NumberDisplay difficultStageHighScoreDisplay;   
    NumberDisplay bossStageHighScoreDisplay;

    std::shared_ptr<SceneComponent> scoreRedirectRoot;      // 反射ステージのハイスコア
    std::shared_ptr<SceneComponent> scoreDifficultRoot;      // 難しいステージのハイスコア
    std::shared_ptr<SceneComponent> scoreBossRoot;      // ボスステージのハイスコア

    DirectX::XMFLOAT3 redirectScorePosition;
    DirectX::XMFLOAT3 difficultScorePosition;
    DirectX::XMFLOAT3 bossScorePosition;

    std::shared_ptr<UIImageComponent> controlAButton;   // Aボタンを表示する
};

