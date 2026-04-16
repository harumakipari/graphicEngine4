#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

enum class YarnEnemyType :uint8_t
{
    Static,   // その場でじっとしている
    MoveHorizontal, // 横に直線移動する
    MoveVertical,   // 縦に直線移動する
    MoveToCenter,   // 中心に向かって移動する
    WaveHorizontal, // 横に波打ちながら移動する
    WaveVertical,  // 縦に波打ちながら移動する
};

class YarnEnemyActor :public Character
{
public:
    explicit YarnEnemyActor(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void TakeDamage(int damage);

    void SetType(YarnEnemyType type);
private:
    // 線形移動の処理
    void MoveLinear(float deltaTime);

    // 中心に向かって移動する処理
    void MoveToCenter(float deltaTime);

    // 横に波打ちながら移動する処理
    void MoveWaveHorizontal(float deltaTime);

    // 縦に波打ちながら移動する処理
    void MoveWaveVertical(float deltaTime);

    // 敵の出現位置によって方向を決定するためのヘルパー関数
    void SetupDirectionFromSpawn();
private:
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;

    YarnEnemyType enemyType = YarnEnemyType::Static;

    // 線形移動のパラメータ
    DirectX::XMFLOAT3 moveDirection = { 1.0f, 0.0f, 0.0f }; // 線形移動の方向
    float speed = 2.0f; // 線形移動の速度

    // 中心に向かって移動するパラメータ
    DirectX::XMFLOAT3 centerPosition = { 6.0f, 0.0f, 6.0f }; // 中心の位置
    DirectX::XMFLOAT3 startPosition = { 0.0f,0.0f,0.0f };   // 中心に向かって移動する前の開始位置
    float moveToCenterSpeed = 1.5f; // 中心に向かって移動する速度
    bool goingToCenter = true; // 中心に向かって移動する途中かどうか
    float reachThreshold = 0.5f; // 中心に到達したとみなす距離の閾値

    // 波打ち移動のパラメータ
    float waveTime = 0.0f;
    float waveAmplitude = 1.0f; // 振れ幅
    float waveFrequency = 3.0f; // 速さ
};
