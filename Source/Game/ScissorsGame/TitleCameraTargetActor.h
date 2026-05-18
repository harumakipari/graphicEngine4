#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"

// タイトルカメラターゲットアクター
class TitleCameraTargetActor :public Actor
{
public:
    explicit TitleCameraTargetActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    // カメラを近づける
    void Play(float interval);

    // カメラを戻す
    void PlayReverse(float interval);

    // タイトル画面に戻る
    void SetTitle(bool isTitle);

private:
    std::unique_ptr<EasingRunner> easingRunner;

    DirectX::XMFLOAT3 titlePosition= { 2.2f,1.984f,2.753f };
    //DirectX::XMFLOAT3 selectPosition= { 0.0f, 2.8f, 2.536f };
    DirectX::XMFLOAT3 selectPosition= { 0.0f,3.1f,2.836f };

    DirectX::XMFLOAT3 startPosition = { 2.2f,1.984f,2.753f };
    DirectX::XMFLOAT3 endPosition = { -0.297f,3.197f,2.936f };

    DirectX::XMFLOAT3 currentPosition = { -0.297f,3.197f,2.936f };

    float easingValue = 0.0f;
};

