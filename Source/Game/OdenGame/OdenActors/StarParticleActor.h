#pragma once
#include "Core/Actor.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "UI/Widgets/Widget.h"



// 　
// 　星のエフェクト表示
//
class StarParticleActor :public Actor
{
public:
    enum class StarPhase :uint8_t
    {
        Orbit,      // 周りを回る
        Merge,      // 合流
        Attract     // スコアへ
    };

    struct TrailStar
    {
        std::shared_ptr<UIImageComponent> sprite;
        float life;
    };

    struct StarAttractInfo
    {
        XMFLOAT3 localOffset;   // 吸い寄せ中のズレ
        float speedFactor;     // 速度の個体差
    };
public:

    explicit StarParticleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Finalize() override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

    // エフェクト再生開始
    void StartParticle(int score);

private:
    void SpawnTrailStar(const XMFLOAT3& worldPos,float intensity);

private:
    std::shared_ptr<EasingRunner> easingRunner;
    std::shared_ptr<ParticleComponent> particleComp;
    static constexpr int StarCount = 5;
    static constexpr int OrbitStarCount = 3;
    std::array<std::shared_ptr<UIImageComponent>, StarCount> starTextures;
    std::array<float, StarCount> starAngles;
    XMFLOAT3 startPos;

    StarAttractInfo attractInfos[StarCount];
    XMFLOAT2 attractStartPos[StarCount];
    XMFLOAT3 scorePos;

    float time = 0.0f;
    float attractDuration = 1.2f;   // スコアへ移動の時間
    float mergeDuration = 0.3f; // 合流時間
    float orbitDuration = 0.6f; // 回転の時間
    float angle = 0.0f;
    float radius = 1.5f;
    StarPhase phase = StarPhase::Orbit;
    XMFLOAT3 prevTrailPos = { 0.0f,0.0f,0.0f };
    std::vector<TrailStar> trailStars;

    float trailInterval = 1.5f;   // 距離間隔
    float trailSpread = 1.5f;  // 横に散らす幅

    int pendingScore = 0.0f; // 加算予定のスコア
};