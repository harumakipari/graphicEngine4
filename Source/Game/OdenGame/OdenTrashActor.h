#pragma once
#include "Core/Actor.h"

// Å@
// Å@ÉSÉ~î†
//
class OdenTrashActor :public Actor
{
public:
    explicit OdenTrashActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

private:
};
