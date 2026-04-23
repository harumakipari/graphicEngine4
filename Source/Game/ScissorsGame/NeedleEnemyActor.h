#pragma once
#include "ScissorsGameEnemyBaseActor.h"

class NeedlePinActor;
class RibbonWallActor;

class NeedleEnemyActor :public ScissorsGameEnemyBase
{
public:
    explicit NeedleEnemyActor(const std::string& actorName) :ScissorsGameEnemyBase(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    // 終了時の処理
    void Finalize() override;

    // 壁を全て壊す
    void BreakAllWalls();

    // 進む方向を設定する
    void SetMoveDirection(DirectX::XMFLOAT3 dir) { moveDirection = dir; }
private:
    // 壁を生成
    void SpawnWall(const DirectX::XMFLOAT3& pos);

    // ステージ端かどうか
    void CheckStageEdge();

private:
    DirectX::XMFLOAT3 lastDropPos = {}; // 最後に壁を置いた位置
    float dropDistance = 0.6f; // この距離進んだら壁置く
    bool isStopped = false; // 止まるかどうか

    // 壁
    std::vector<std::shared_ptr<RibbonWallActor>> walls;
    // 待ち針のピン
    std::shared_ptr<NeedlePinActor> needlePinActor;
};


class NeedlePinActor :public Actor
{
public:
    explicit NeedlePinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

private:
    std::shared_ptr<SkeletalMeshComponent> needlePinComponent; // 待ち針のモデル
};