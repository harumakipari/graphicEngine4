#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"


class ButtonBombActor :public Actor
{
public:
    explicit ButtonBombActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    // 爆発処理
    void Explode();
private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;

};
