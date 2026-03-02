#include "pch.h"
#include "Camera.h"

#include "Components/Controller/ControllerComponent.h"

void MainCamera::Update(float deltaTime)
{

    if (auto target = mainCameraComponent->target.lock())
    {
        auto actor = target->GetOwner();
        
    }





    // プレイヤーの移動方向を取得
    XMFLOAT3 moveDir = {};

    if (auto target = mainCameraComponent->target.lock())
    {
        auto actor = target->GetOwner();

        if (auto movement = actor->GetComponent<CharacterMovementComponent>())
        {
            moveDir = movement->GetVelocity();
        }
    }

    // 右スティック
    XMFLOAT2 rightStick =
        InputSystem::GetRightStick();

    // AutoFollow 呼び出し
    //static_cast<TPSCameraComponent*>(mainCameraComponent.get())->AutoFollow(moveDir, rightStick, deltaTime);
}

