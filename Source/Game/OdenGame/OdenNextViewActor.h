#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"



// 　
// 　次の食材の表示
//
class OdenNextViewActor :public Actor
{
public:
    explicit OdenNextViewActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override {}

private:
    // 表示枠（3つ）
    std::array<std::shared_ptr<UIImageComponent>, 3> nextSlots;

    // 名前 → テクスチャ
    std::unordered_map<std::string, std::shared_ptr<Sprite>> ingredientTextures;

};
