#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"


class ButtonBombActor :public Actor
{
    enum class BombState :uint8_t
    {
        Preparing,  // 準備中
        Falling,    // 落下中
        Waiting,    // 地面で待機
        Blinking,   // 点滅
        Exploded    // 爆発済み
    };

public:
    explicit ButtonBombActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

    // 爆発処理
    void Explode();

    // ボス位置から爆弾位置まで発射する処理
    void LaunchTo(const DirectX::XMFLOAT3& targetPos);
private:
    // 爆発エフェクトを再生する
    void PlayBombEffect(DirectX::XMFLOAT3 pos);


private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    // 爆発パーティクルコンポーネントを追加
    std::shared_ptr<ParticleComponent> bombEffectComponent;

    BombState bombState = BombState::Preparing;
    float elapsedTime;    // 経過時間
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };
    float gravity = 4.9f;
    float groundY = 0.0f;   // 地面の基準点
    bool hasExploded = false;   // 爆発したかどうか

    // 調整値
    float explodeRange = 1.5f; // 爆発影響範囲
    float blinkDelay = 0.3f;     // 点滅が始まるまで
    float explodeDelay = 0.5f;   // 爆発するまで

};
