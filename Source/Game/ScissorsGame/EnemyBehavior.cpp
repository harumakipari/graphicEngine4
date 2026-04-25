#include "pch.h"
#include "EnemyBehavior.h"
#include "EnemyBase.h"
#include "ScissorsGameState.h"


void LinearBehavior::Enter(EnemyBase* e)
{
    
}

void LinearBehavior::Update(EnemyBase* e, float deltaTime)
{
    DirectX::XMFLOAT3 pos = e->GetPosition();
    DirectX::XMFLOAT3 moveDirection = e->GetMoveDirection();
    float speed = e->GetSpeed();

    pos.x += moveDirection.x * speed * deltaTime;
    pos.z += moveDirection.z * speed * deltaTime;

    // ステージ端で反転
    if (pos.x < ScissorsGameState::stageMinX || pos.x > ScissorsGameState::stageMaxX)
    {
        moveDirection.x *= -1.0f;
        pos.x = std::clamp(pos.x, ScissorsGameState::stageMinX, ScissorsGameState::stageMaxX);
    }

    if (pos.z < ScissorsGameState::stageMinZ || pos.z > ScissorsGameState::stageMaxZ)
    {
        moveDirection.z *= -1.0f;
        pos.z = std::clamp(pos.z, ScissorsGameState::stageMinZ, ScissorsGameState::stageMaxZ);
    }

    e->SetPosition(pos);
    e->SetMoveDirection(moveDirection);

    e->Face(moveDirection);
}

void LinearBehavior::Exit(EnemyBase* e)
{
    
}
