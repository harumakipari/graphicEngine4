#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

// 　
// 　タイマーのUI表示
//
class GameTimerUiActor :public Actor
{
public:
    explicit GameTimerUiActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    void UpdateTimerDigits(int totalSeconds);

private:
    std::vector<std::shared_ptr<UIImageComponent>> timerDigits;

    std::shared_ptr<UIImageComponent> timerFrameImage;
};
