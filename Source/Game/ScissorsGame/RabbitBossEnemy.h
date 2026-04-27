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

    void Update(float elapsedTime)override;

    //void EnlargeRandomEnemies(int count);

};

