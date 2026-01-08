#pragma once
#include "Core/Actor.h"
#include "OdenDataStruct.h"
#include "UI/Widgets/Widget.h"


struct OrderData
{
    EOdenOrderShape requiredShape;
};

// 　お題
// 　ふきだし
//
class OrderBubbleActor :public Actor
{
public:
    explicit OrderBubbleActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override {}

    void Update(float elapsedTime)override {}

private:
    std::shared_ptr<UIImageComponent> orderUi; // オーダーの吹き出し
};
