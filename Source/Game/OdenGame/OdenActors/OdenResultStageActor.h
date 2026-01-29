#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"


// 　タイトル店
// 　モデル
//
class OdenResultStageActor :public Actor
{
public:
    OdenResultStageActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override {}

    void DrawImGuiDetails() override;

private:
    // 当たり判定を切り替える
    void SwitchCollision();

private:
    std::shared_ptr<StaticMeshComponent> storeModelComponent;     // 店のモデル
    std::shared_ptr<StaticMeshComponent> soupModelComponent; // 汁のモデル

    std::shared_ptr<BoxComponent> rightBoxComponent;    // 右の当たり判定
    std::shared_ptr<BoxComponent> rightAfterBoxComponent;    // 右の当たり判定

};
