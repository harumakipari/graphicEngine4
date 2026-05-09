#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

// ボーナスUIアクター
class BonusUiActor :public Actor
{
public:
    explicit BonusUiActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

private:
    std::shared_ptr<UIImageComponent> bonusUiComponent;
    float elapsedTime = 0.0f;
};

