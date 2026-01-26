#pragma once
#include "Core/Actor.h"
#include "OdenData/OdenDataStruct.h"

class OdenResultIngredientActor;

// おでんの串モデル
class OdenResultSkewerActor : public Actor
{
public:
    explicit OdenResultSkewerActor(const std::string& name) : Actor(name) {}

    void Initialize(const Transform& transform)override;

    void AddIngredient(const std::shared_ptr<OdenResultIngredientActor>& ingredient, int index);

private:
    // 食材の種類によってオフセットを取得する
    float GetIngredientYOffset(EOdenType type);

private:
    std::shared_ptr<SkeletalMeshComponent> poleModel;
    std::array<std::shared_ptr<OdenResultIngredientActor>, 3> ingredientArray;
    std::vector<std::shared_ptr<OdenResultIngredientActor>> ingredients;
};
