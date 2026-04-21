#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "./Core/Actor.h"



class ButtonCoinActor :public Actor
{
public:
    explicit ButtonCoinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // 演出開始する
    void StartPerform();

private:
    std::shared_ptr<ParticleComponent> particleComponent; 

    float elapsedTime = 0.0f; // 経過時間
    DirectX::XMFLOAT3 startPos = { 0,0,0 }; // 開始位置
    float duration = 1.5f; // 演出に掛ける時間
    float height = 2.0f;

    bool startPerform = false; // 演出を開始するかどうか
};
