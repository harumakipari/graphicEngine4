#pragma once
#include "ScissorsGameEnemyBaseActor.h"

class NeedleEnemyActor :public ScissorsGameEnemyBase
{
public:
    explicit NeedleEnemyActor(const std::string& actorName) :ScissorsGameEnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

private:
    // 壁を生成
    void SpawnWall(const DirectX::XMFLOAT3& pos);

private:
    DirectX::XMFLOAT3 lastDropPos = {}; // 最後に壁を置いた位置
    float dropDistance = 0.6f; // この距離進んだら壁置く
    std::vector<std::shared_ptr<Actor>> walls;

};