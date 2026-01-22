#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"

// 　
// 　リザルトで描画する食材
//
class OdenResultIngredientActor :public Actor
{
public:
    explicit OdenResultIngredientActor(const std::string& actorName, const std::string& ingredientName) :Actor(actorName),ingredientName(ingredientName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // 食材が登場する
    void AppearIngredient();

    // フィーバーモードで提出された具材かどうかを設定する
    void SetFeverModeIngredient(bool isFever)
    {
        isPlayTwinkleEffect = isFever;
    }

    // フィーバーモードで提出された具材かどうかを取得する
    bool GetIsFeverModeIngredient() const
    {
        return isPlayTwinkleEffect;
    }
private:
    std::string ingredientName; // 食材の名前
    std::shared_ptr<SkeletalMeshComponent> ingredientModel; // 具材
    std::shared_ptr<ParticleComponent> particleComponent;   // エフェクト
    std::shared_ptr<ParticleComponent> twinkleParticleComponent;   // エフェクト

    // 経過時間
    float elapsedTime = 0.0f;
    float modelSpawnTime = 0.5f; // モデルが表示されるまでの時間

    // エフェクトを再生したかどうか
    bool isPlayEffect = false;

    // キラキラエフェクトを再生するかどうか
    bool isPlayTwinkleEffect = false;

};
