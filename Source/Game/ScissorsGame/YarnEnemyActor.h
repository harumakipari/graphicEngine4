#pragma once
#include "EnemyScoreData.h"
#include "ScissorsGameEnemyBaseActor.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class ScissorsPlayer1;

class YarnEnemyActor :public ScissorsGameEnemyBase
{
public:
    explicit YarnEnemyActor(const std::string& actorName) :ScissorsGameEnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void SetType(YarnEnemyType type);



private:
    // 中心に向かって移動する処理
    void MoveToCenter(float deltaTime);

    // 横に波打ちながら移動する処理
    void MoveWaveHorizontal(float deltaTime);

    // 縦に波打ちながら移動する処理
    void MoveWaveVertical(float deltaTime);

    // プレイヤーを追いかける処理
    void ChasePlayer(float deltaTime);


private:

    // 中心に向かって移動するパラメータ
    DirectX::XMFLOAT3 centerPosition = { 6.0f, 0.0f, 6.0f }; // 中心の位置
    bool goingToCenter = true; // 中心に向かって移動する途中かどうか
    float reachThreshold = 0.5f; // 中心に到達したとみなす距離の閾値

    // 波打ち移動のパラメータ
    float waveTime = 0.0f;
    float waveAmplitude = 1.0f; // 振れ幅
    float waveFrequency = 3.0f; // 速さ


};

class BigYarnEnemyActor :public YarnEnemyActor
{
public:
    explicit BigYarnEnemyActor(const std::string& actorName) :YarnEnemyActor(actorName) {}
    void Initialize(const Transform& transform)override;
    // プレイヤーのダッシュに当たったときの処理
    bool OnHitByDash(ScissorsPlayer1* player, int dashDamage)override;

private:

};