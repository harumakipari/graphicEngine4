#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"


class ButtonBombActor :public Actor
{
    enum class BombState :uint8_t
    {
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
private:
    // 爆発エフェクトを再生する
    void PlayBombEffect(DirectX::XMFLOAT3 pos);


private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    // 爆発パーティクルコンポーネントを追加
    std::shared_ptr<ParticleComponent> bombEffectComponent;


    BombState bombState = BombState::Falling;
    float elapsedTime;    // 経過時間
    bool hasExploded = false;   // 爆発したかどうか

    // 調整値
    float explodeRange = 2.5f; // 爆発影響範囲
    float blinkDelay = 2.0f;     // 点滅が始まるまで
    float explodeDelay = 3.0f;   // 爆発するまで

};
