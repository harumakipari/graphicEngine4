#include "pch.h"
#include "Camera.h"

#include "Components/Controller/ControllerComponent.h"
#include "Engine/Scene/SceneBase.h"

void Camera::Initialize(const Transform& transform)
{
    mainCameraComponent = this->AddComponent<class TPSCameraComponent>("mainCamera");
    mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(35), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
}

void MainCamera::Update(float deltaTime)
{
    // プレイヤー移動方向
    XMFLOAT3 moveDir = {};

    if (auto target = tpsController.target.lock())
    {
        auto actor = target->GetOwner();

        if (auto movement = actor->GetComponent<CharacterMovementComponent>())
        {
            moveDir = movement->GetVelocity();
        }
    }

    // 右スティック
    XMFLOAT2 rightStick = InputSystem::GetRightStick();

    // カメラ回転
    mainCameraComponent->AddYaw(rightStick.x * deltaTime * 2.0f);
    mainCameraComponent->AddPitch( -rightStick.y * deltaTime * 2.0f);

    //const float limit = DirectX::XMConvertToRadians(80.0f);

    //mainCameraComponent->pitch =
    //    std::clamp(
    //        mainCameraComponent->pitch,
    //        -limit,
    //        limit
    //    );

    // Controller更新
    tpsController.Update(deltaTime);

}

