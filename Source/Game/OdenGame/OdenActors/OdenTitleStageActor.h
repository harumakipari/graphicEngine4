#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"


// 　タイトル店
// 　モデル
//
class OdenTitleStageActor :public Actor
{
public:
    OdenTitleStageActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

private:
    std::shared_ptr<StaticMeshComponent> storeModelComponent;     // 店のモデル
    std::shared_ptr<SkeletalMeshComponent> selectModelComponent;   // 難易度選択のモデル

};
