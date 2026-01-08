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

    void Initialize(const Transform& transform)override {}

    void Update(float elapsedTime)override {}

    // ビート毎に回転する
    void OnBeat() const;

    ERotationType rotationType = ERotationType::Horizontal;

private:
    std::weak_ptr<OdenIngredientActor> odenIngredientActor; 
};
