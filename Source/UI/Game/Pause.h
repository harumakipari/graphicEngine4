#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

class Pause :public Actor
{
public:
    enum class PauseState:uint8_t
    {
        Playing,
        Paused,
        ResumeCountdown, 
    };

public:
    explicit Pause(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<UIImageComponent> pausePanel;
    std::shared_ptr<UIButtonComponent> menuButton;
    std::shared_ptr<UIButtonComponent> returnTitleButton;
    std::shared_ptr<UIButtonComponent> closeButton;

    std::array<std::shared_ptr<UIImageComponent>, 3> countDownImages;
    PauseState state = PauseState::Playing;
    float countdownTime = 3.0f;
    int lastCountdownNumber = -1;
};
