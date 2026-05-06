#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"


class ButtonBombActor :public Actor
{
public:
    explicit ButtonBombActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 爆発処理
    void Explode();
private:
    // 爆発エフェクトを再生する
    void PlayBombEffect(DirectX::XMFLOAT3 pos);


private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    float explodeRange = 2.5f; // 爆発影響範囲
    std::shared_ptr<ParticleComponent> bombEffectComponent;


};
