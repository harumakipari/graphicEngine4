#pragma once
#include "EnemyScoreData.h"
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"

class BobbinActor :public Actor
{
public:
    explicit BobbinActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    //　機能をリセットする
    void Reset();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;// 描画用コンポーネントを追加

    bool isActivated = false; // 使用フラグ
    float currentRadius = 0.0f;


    // 調整
    float expandSpeed = 2.0f; // 広がる速度
    float maxRadius = 6.0f; // 最大半径
};

