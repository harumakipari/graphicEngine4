#pragma once
#include "Core/Actor.h"

// 飛ばす針
class NeedleActor :public Actor
{
public:
    explicit NeedleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // 飛ばす最終目的位置
    void SetTargetPos(const DirectX::XMFLOAT3& targetPos);

private:
    // 刺さる処理
    void Stick(); 

private:
    DirectX::XMFLOAT3 targetPos = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };
    float speed = 10.0f;
    float radius = 0.2f;
    std::shared_ptr<SkeletalMeshComponent> mesh;
    std::shared_ptr<SphereComponent> collision;
    bool isStuck = false; // 地面に付いたかどうか
    float lifeTimer = 0.0f;
    float lifeTimeInterval = 0.5f; // 地面についてから消えるまでの時間
};

