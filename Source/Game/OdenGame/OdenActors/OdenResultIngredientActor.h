#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

class RotationComponent;
class ParticleComponent;
class CoreAudioSourceComponent;

// 　
// 　リザルトで描画する食材
//
class OdenResultIngredientActor :public Actor
{
public:
    explicit OdenResultIngredientActor(const std::string& actorName, const std::string& ingredientName) :Actor(actorName), ingredientName(ingredientName) {}

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


    void SetRelativePosition(const DirectX::XMFLOAT3& pos) const
    {
        if (ingredientModel)
            ingredientModel->SetRelativeLocationDirect(pos);
    }

    DirectX::XMFLOAT3 GetModelSize() const
    {
        if (ingredientModel)
            return ingredientModel->GetModelSize();
        return { 1.0f, 1.0f, 1.0f };
    }

    // 串内のインデックス
    void SetIndexInSkewer(int index);

    // 食材の種類を取得する
    EOdenType GetIngredientType() const { return  ingredientType; }
private:
    std::string ingredientName; // 食材の名前
    std::shared_ptr<SkeletalMeshComponent> ingredientModel; // 具材
    std::shared_ptr<ParticleComponent> particleComponent;   // エフェクト
    std::shared_ptr<ParticleComponent> twinkleParticleComponent;   // エフェクト
    std::shared_ptr<CoreAudioSourceComponent> audioComponent;   // エフェクト
    std::shared_ptr<CoreEasingComponent> easingComponent;
    std::shared_ptr<RotationComponent> rotationComponent;

    // 経過時間
    float elapsedTime = 0.0f;
    float modelSpawnTime = 0.4f; // モデルが表示されるまでの時間

    // エフェクトを再生したかどうか
    bool isPlayEffect = false;

    // キラキラエフェクトを再生するかどうか
    bool isPlayTwinkleEffect = false;

    // モデルの動きのための時間
    float totalTime = 0.0f;

    // 串内のインデックス
    int indexInSkewer = 0;

    EOdenType ingredientType = EOdenType::Daikon;
};
