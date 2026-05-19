#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Game/Actors/Base/Character.h"


class TitlePlayerActor :public Character
{
public:
    explicit TitlePlayerActor(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override {}

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加
};

