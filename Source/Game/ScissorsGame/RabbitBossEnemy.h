#pragma once
#include "EnemyScoreData.h"
#include "ScissorsGameEnemyBaseActor.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"


class RabbitBossEnemyActor :public ScissorsGameEnemyBase
{
public:
    explicit RabbitBossEnemyActor(const std::string& actorName) :ScissorsGameEnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ƒ‰ƒ“ƒ_ƒ€‚É‘å‚«‚¢“G‚É•ÏX‚·‚éˆ—
    void EnlargeRandomEnemies(int count);

private:
    float attackTimer = 0.0f;
    const float attackTimeInterval = 5.0f; // UŒ‚‚ÌŠÔŠu
};

