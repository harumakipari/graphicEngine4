#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"

class OdenIngredientActor;

// 枠の回転のタイプ
enum class ERotationType :uint8_t
{
    Horizontal,
    Vertical
};

// 　枠組
// 　回転・配置
//
class OdenSlotActor :public Actor
{
public:
    OdenSlotActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    // 食材をセットする
    void SetIngredient(const std::shared_ptr<OdenIngredientActor>& newIngredient);

    // 食材を外す
    std::shared_ptr<OdenIngredientActor> RemoveIngredient();

    // 食材の種類を取得する
    std::shared_ptr<OdenIngredientActor> GetIngredient() const;

    // ビート毎に回転する
    void OnBeat() const;

    // ビートに合わせてスケールを変える
    void SetVisualScale(float scale);

    ERotationType rotationType = ERotationType::Horizontal;

private:
    std::shared_ptr<OdenIngredientActor> odenIngredientActor;   // スロットの中のおでんの食材
    std::shared_ptr<ParticleComponent> particleComponent;   // 湯気のエフェクト
};
