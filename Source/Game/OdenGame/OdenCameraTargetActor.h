#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"


// 　カメラのターゲット
// 
class OdenCameraTargetActor :public Actor
{
public:
    OdenCameraTargetActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // ターゲットの移動開始  何秒で移動するか
    void PlayToTarget(float moveTime);

    // ターゲットの移動開始  何秒で移動するか
    void PlayToOrigin(float moveTime);
private:
    void StartEasing(float moveTime, float from, float to);

public:
    std::function<void()> onMoveStarted;
    std::function<void()> onMoveFinished;
private:
    std::shared_ptr<CoreEasingComponent> easingComponent;

    //DirectX::XMFLOAT3 originPos = { -12.3f,13.8f,-12.5f };
    DirectX::XMFLOAT3 originPos = { -18.3f,13.8f,-19.6f };
    DirectX::XMFLOAT3 targetPos = { -3.6f,5.7f,0.3f };
    float moveTimer = 3.0f;
    float easingValue = 0.0f;
};
