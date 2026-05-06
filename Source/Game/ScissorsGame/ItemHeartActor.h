#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"


class ItemHeartActor :public Actor
{
public:
    explicit ItemHeartActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

private:
    // アイテムを使用する
    void UseItem();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加

};

