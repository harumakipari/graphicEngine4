#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"

// ゲームカメラターゲットアクター
class GameCameraTargetActor :public Actor
{
public:
    explicit GameCameraTargetActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // カメラを近づける
    void Play(float interval);

private:
    std::unique_ptr<EasingRunner> easingRunner;

    float easingValue = 0.0f;

    DirectX::XMFLOAT3 startPosition = { 2.2f,1.984f,2.753f };
    DirectX::XMFLOAT3 endPosition = { -0.297f,3.197f,2.936f };
    DirectX::XMFLOAT3 currentPosition = { -0.297f,3.197f,2.936f };

    DirectX::XMFLOAT3 bossOffSetPos = { 0.0f,3.5f,-5.0f };

};

