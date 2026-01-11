#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenData/OdenDataStruct.h"
#include "Game/OdenGame/OdenData/OdenShapeDataTable.h"

class OdenBubbleActor;

// ‚¨‹q‚³‚ñ‚ğ•À‚×‚é
class OdenOrderManager :public Actor
{
public:
    OdenOrderManager(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override {}

private:
    // ‚¨‹q‚³‚ñ‚ğoŒ»‚³‚¹‚é
    void SpawnOrderBubble(int index);

    // ƒ‰ƒ“ƒ_ƒ€‚É‚¨‘è‚ğ¶¬‚·‚é
    OrderEntry PickRandomOrder();

    // ‡”Ô‚©‚çˆÊ’u‚ğæ“¾‚·‚é
    DirectX::XMFLOAT3 GetBubblePosition(const int index);

    // ’•¶‚ªŠ®—¹‚µ‚½‚ÉŒÄ‚Î‚ê‚éŠÖ”
    void OnBubbleCompleted(const OdenBubbleActor& bubble, float score);

    // •À‚Ñ‚ğ‹l‚ß‚é
    void RearrangeBubbles();
private:
    std::vector<std::weak_ptr<OdenBubbleActor>> bubbles;

    // Å‘å‚Ì’•¶‚ğ•·‚¯‚él”
    static constexpr int MaxOrders = 3;
    DirectX::XMFLOAT3 basePos = { 2.0f,3.0f,9.0f };
    float spacing = 3.0f;

};