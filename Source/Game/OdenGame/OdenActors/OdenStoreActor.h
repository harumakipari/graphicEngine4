#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"


// 　店
// 　モデル
//
class OdenStoreActor :public Actor,public IBeatReactive
{
public:
    OdenStoreActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

    // 拍が来た瞬間の処理
    void OnBeat(bool isStrong) override {}

    // 毎フレーム呼ばれる (0 ~ 1)
    void OnBeatPhase(float phase) override;

private:
    std::shared_ptr<StaticMeshComponent> storeModelComponent;     // 店のモデル

};
