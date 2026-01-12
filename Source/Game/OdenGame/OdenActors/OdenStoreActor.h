#pragma once
#include "Core/Actor.h"


// Å@ìX
// Å@ÉÇÉfÉã
//
class OdenStoreActor :public Actor
{
public:
    OdenStoreActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

private:
    

};
