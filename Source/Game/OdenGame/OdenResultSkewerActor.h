#pragma once
#include "Components/Easing/CoreEasingComponent.h"
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

    // 回転開始
    void StartRotateOneTurn();

private:
    // 食材の種類によってオフセットを取得する
    float GetIngredientYOffset(EOdenType type);
public:
    std::function<void()> onRotationFinished;
    std::shared_ptr<SkeletalMeshComponent> poleModel;
    std::vector<std::shared_ptr<OdenResultIngredientActor>> ingredients;

private:
    std::array<std::shared_ptr<OdenResultIngredientActor>, 3> ingredientArray;
    std::shared_ptr<CoreEasingComponent> easingRunner;

    DirectX::XMFLOAT4 startOrientation = { 0.0f,0.0f,0.0f,1.0f }; // 初期姿勢
};
