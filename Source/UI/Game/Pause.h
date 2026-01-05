#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

class Pause :public Actor
{
public:
    Pause(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

private:
    std::shared_ptr<UIImageComponent> pausePanel;
    std::shared_ptr<UIButtonComponent> menuButton;
    std::shared_ptr<UIButtonComponent> returnTitleButton;
    std::shared_ptr<UIButtonComponent> closeButton;
};
