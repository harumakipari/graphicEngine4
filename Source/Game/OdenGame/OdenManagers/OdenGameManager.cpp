#include "pch.h"
#include "OdenGameManager.h"

void OdenGameManager::Initialize(const Transform& transform)
{
    // タイマーやスコアをリセットする
    Reset();
}

void OdenGameManager::Update(float deltaTime)
{
    remainingTime -= deltaTime;
    remainingTime = std::max<float>(remainingTime, 0.0f);


}

// ゲームのステートをリセットする
void OdenGameManager::Reset()
{
    totalScore = 0;
    combo = 0;
    remainingTime = maxTime;
}

