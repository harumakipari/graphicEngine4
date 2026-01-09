#pragma once
#include "Game/OdenGame/OdenData/OdenDataStruct.h"

class OdenSlotActor;
class OdenBubbleActor;

// 全体の管理
// スコア管理
// 次のおでん補充
// ビート管理
// ゲーム進行

class OdenGameManager
{
public:
    int score;  // ゲーム全体のスコア
    std::queue<EOdenType> nextOdenQueue;
    std::vector<OdenSlotActor*> slots;
    std::vector<OdenBubbleActor*> orders;
};