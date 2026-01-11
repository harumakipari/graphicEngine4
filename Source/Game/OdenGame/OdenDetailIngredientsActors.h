#pragma once
#include "Core/Actor.h"
#include "OdenIngredientActor.h"

// それぞれの食材

// ダイコン
class OdenDaikonActor : public OdenIngredientActor
{
public:
    OdenDaikonActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};

// こんにゃく
class OdenKonnyakuActor : public OdenIngredientActor
{
public:
    OdenKonnyakuActor(const std::string& actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;
};
