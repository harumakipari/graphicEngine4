#pragma once
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/StaticMeshCollisionComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"




// ãÔçﬁ
// ÅiâÒÇÈÅEíÕÇﬂÇÈÅj
//
class OdenIngredientActor :public Actor
{
public:
    OdenIngredientActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override {}

    void Update(float elapsedTime)override {}

    //void SetAngleOffset()

protected:
    std::shared_ptr<SkeletalMeshComponent> ingredientModel; // ãÔçﬁ

};


class OdenDaikonActor : public OdenIngredientActor
{
public:
    OdenDaikonActor(const std::string & actorName) :OdenIngredientActor(actorName) {}

    void Initialize(const Transform & transform)override
    {
        std::string parentName = "Daikon_model";
        ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
        ingredientModel->SetModel("./Data/Models/Oden_Ingredient/Oden_Daikon.glb");

    }

    void Update(float elapsedTime)override {}
};