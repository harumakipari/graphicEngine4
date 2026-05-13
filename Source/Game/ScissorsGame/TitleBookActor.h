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

private:
    NumberDisplay firstStageHighScoreDisplay;   
    NumberDisplay bossStageHighScoreDisplay;   

};

