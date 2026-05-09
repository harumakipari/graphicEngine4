#pragma once
#include "Components/Effect/ParticleComponent.h"
#include "./Core/Actor.h"


class UIStarEffect;

class ButtonCoinActor :public Actor
{
    enum class CoinState :uint8_t
    {
        Before,
        Rising,
        Burst,
        Finished
    };
    CoinState state = CoinState::Before;

public:
    explicit ButtonCoinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

    void Finalize()override;

    // 演出開始する
    void StartPerform(bool isBonus);

    
private:
    // きらきらバースト
    void SpawnBurst();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<ParticleComponent> particleComponent;

    float elapsedTime = 0.0f; // 経過時間
    DirectX::XMFLOAT3 startPos = { 0,0,0 }; // 開始位置

    // ===== 上昇トレイル =====
    float trailTimer = 0.0f;

    // ===== バースト =====
    int burstCount = 8;
    float burstRadius = 80.0f;
    float burstSize = 100.0f;
    float burstShrinkSpeed = 120.0f;


    std::vector<std::shared_ptr<UIStarEffect>> starEffects;

    bool isBonus = false; // ボーナスコインかどうか
};
