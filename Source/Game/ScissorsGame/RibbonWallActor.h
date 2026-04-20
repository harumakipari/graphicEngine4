#pragma once
#include "./Core/Actor.h"

class NeedleEnemyActor;

class RibbonWallActor :public Actor
{
public:
    explicit RibbonWallActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override{}

    // •Ç‚ğ‰ó‚·
    void Break();
    
public:
    // ‚±‚Ì•Ç‚ğŠ—L‚µ‚Ä‚¢‚é“G
    std::weak_ptr<NeedleEnemyActor> ownerEnemy;

private:
    // •Ç‚Ì“–‚½‚è”»’è
    std::shared_ptr<SphereComponent> wallCollisionComponent;
};
