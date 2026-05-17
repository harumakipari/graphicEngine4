#pragma once
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"


class HighScoreMedalActor :public Actor
{
public:
    explicit HighScoreMedalActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 終了時の処理
    void Finalize() override;

    // 演出開始
    void Play();

    // ターゲット位置を設定
    void SetTarget(const DirectX::XMFLOAT3 pos) { targetPos = pos; }

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::unique_ptr<EasingRunner> easingRunner;  // 一ページ目のeasingComponent

    DirectX::XMFLOAT3 targetPos = { 0.0f,0.0f,0.0f };

    float startScale = 3.0f;
    float endScale = 1.0f;

    float currentScale = 1.0f;

    float medalValue = 0.0f;
    float interval = 0.5f; // メダルが移動する時間
};


