#pragma once
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"

class ScissorsPlayer1;

class ScissorsGameEnemyBase :public Character
{
public:
    explicit ScissorsGameEnemyBase(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override{}

    // ダメージを与える　死亡したかどうかを取得する関数
    virtual bool TakeDamage(int damage);

    // 移動の方向を設定する関数
    void SetMoveDirection(const DirectX::XMFLOAT3& dir)
    {
        moveDirection = dir;
    }
    // 速度を設定する関数
    void SetSpeed(float speed)
    {
        this->speed = speed;
    }

    // プレイヤーのダッシュに当たったときの処理
    virtual bool OnHitByDash(ScissorsPlayer1* player, int dashDamage);

    // 敵のスコアを取得する関数
    EnemyScoreData GetScoreData() const { return scoreData; }


protected:
    // 線形移動の処理
    void MoveLinear(float deltaTime);

public:
    // 死亡通知
    std::function<void()> onDeath;


protected:
    DirectX::XMFLOAT3 startPosition = { 0.0f,0.0f,0.0f };   // 中心に向かって移動する前の開始位置

    // 描画用コンポーネントを追加
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<ParticleComponent> starEffectComponent;

    YarnEnemyType enemyType = YarnEnemyType::Static;
    // 移動のパラメータ
    DirectX::XMFLOAT3 moveDirection = { 1.0f, 0.0f, 0.0f }; // 線形移動の方向
    float speed = 2.0f; // 線形移動の速度


    float enemyRadius = 0.5f; // 敵の当たり判定
    int maxHp = 1;
    EnemyScoreData scoreData={ 100,0 }; // 倒したときのスコア


};