#pragma once
#include "Core/Actor.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "UI/Widgets/Widget.h"

class ScaleTransitionEffect :public Actor
{
public:
    explicit ScaleTransitionEffect(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    bool IsFinished() const { return isFinishTransitionPerform; }

private:
    std::shared_ptr<UIImageComponent> sprite;
    std::shared_ptr<CoreEasingComponent> easingComponent;
    float time = 0.0f;
    float spriteScale = 1.0f;
    bool isFinishTransitionPerform = false;
};

