#pragma once
#include "./Core/Actor.h"

class RibbonWallActor :public Actor
{
public:
    explicit RibbonWallActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

private:
    // •Ç‚Ì“–‚½‚è”»’è
    std::shared_ptr<SphereComponent> wallCollisionComponent;

};