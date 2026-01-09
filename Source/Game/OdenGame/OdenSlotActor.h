#pragma once
#include "Core/Actor.h"

class OdenIngredientActor;

// 枠の回転のタイプ
enum class ERotationType :uint8_t
{
    Horizontal,
    Vertical
};


struct SlotData 
{
    ERotationType rotationType;
    float rotateInterval; // リズム用
};

// 　枠組
// 　回転・配置
//
class OdenSlotActor :public Actor
{
public:
    OdenSlotActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

    // 食材をセットする
    void SetIngredient(const std::shared_ptr<OdenIngredientActor>& newIngredient);

    // 食材を外す
    std::shared_ptr<OdenIngredientActor> RemoveIngredient();

    // 食材の種類を取得する
    std::shared_ptr<OdenIngredientActor> GetIngredient() const;

    // ビート毎に回転する
    void OnBeat() const;

    ERotationType rotationType = ERotationType::Horizontal;

private:
    std::shared_ptr<OdenIngredientActor> odenIngredientActor; 
};
