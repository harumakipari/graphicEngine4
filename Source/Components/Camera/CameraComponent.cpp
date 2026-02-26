#include "pch.h"
#include "CameraComponent.h"

#include "Core/Actor.h"


const DirectX::XMFLOAT4X4& TPSCameraComponent::GetView() 
{
    using namespace DirectX;

    XMFLOAT3 basePos{ 0,0,0 };
    if (!target.expired())
    {
        basePos = target.lock()->GetComponentWorldTransform().GetLocation();
    }
    else
    {
        
    }
    XMVECTOR focus =
        XMLoadFloat3(&basePos) +
        XMVectorSet(targetOffset.x, targetOffset.y, targetOffset.z, 0);

    // yaw/pitch から forward を直接作る
    XMVECTOR forward =
        XMVector3Normalize(
            XMVectorSet(
                cosf(pitch) * sinf(yaw),
                sinf(pitch),
                cosf(pitch) * cosf(yaw),
                0
            )
        );

    // カメラ位置
    XMVECTOR eye = focus - forward * distance;
    DirectX::XMFLOAT3 eye3;
    DirectX::XMStoreFloat3(&eye3, eye);
    if (auto owner = owner_.lock())
    {
        owner->SetPosition(eye3);
    }

    XMStoreFloat4x4(
        &view,
        XMMatrixLookAtLH(
            eye,
            focus,
            XMVectorSet(0, 1, 0, 0)
        )
    );

    return view;


}

// 自動随従する
void TPSCameraComponent::AutoFollow(const DirectX::XMFLOAT3& moveDir, const DirectX::XMFLOAT2& rightStick, float deltaTime)
{
    using namespace DirectX;

    // 右スティック触ったらタイマーリセット
    float stickMag =
        sqrtf(rightStick.x * rightStick.x +
            rightStick.y * rightStick.y);

    if (stickMag > autoFollowDeadZone)
    {
        autoFollowDelayTimer = autoFollowDelay;
        return;
    }

    // タイマー中は追従しない
    if (autoFollowDelayTimer > 0.0f)
    {
        autoFollowDelayTimer -= deltaTime;
        return;
    }

    // 移動していない
    float len =
        sqrtf(moveDir.x * moveDir.x +
            moveDir.z * moveDir.z);

    if (len < 0.1f)
        return;

    // 正規化
    XMFLOAT3 dir = {
        moveDir.x / len,
        0.0f,
        moveDir.z / len
    };

    float desiredYaw =
        atan2f(dir.x, dir.z);

    float diff =
        WrapAngle(desiredYaw - yaw);

    // 小さい角度は無視
    if (fabsf(diff) < XMConvertToRadians(90.0f))
        return;

    // ゆっくり回す
    yaw += diff *
        autoFollowStrength *
        deltaTime;
}


void DebugCameraComponent::HandleKeyboardInput(float deltaTime)
{
    using namespace DirectX;
    DirectX::XMFLOAT4 rotaion = GetComponentRotation();
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
    DirectX::XMVECTOR forward = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), rotationMatrix);
    DirectX::XMVECTOR right = rotationMatrix.r[0];
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

    DirectX::XMVECTOR move = DirectX::XMVectorZero();
#ifdef USE_IMGUI

    if (float wheelDelta = ImGui::GetIO().MouseWheel)
    {
        move += forward * wheelDelta * 30.0f;
    }
#endif
    if (InputSystem::GetInputState("W")) { move += forward; }
    if (InputSystem::GetInputState("S")) { move -= forward; }
    if (InputSystem::GetInputState("D")) { move += right; }
    if (InputSystem::GetInputState("A")) { move -= right; }
    //
    if (InputSystem::GetInputState("E")) { move += up; }
    if (InputSystem::GetInputState("Q")) { move -= up; }

    if (InputSystem::GetInputState("Shift")) { move = DirectX::XMVectorScale(move, 2.5f); }

    move = DirectX::XMVectorScale(move, moveSpeed * deltaTime);

    DirectX::XMFLOAT3 position = GetComponentLocation();
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos += move;
    DirectX::XMFLOAT3 positionLocal{};
    DirectX::XMStoreFloat3(&positionLocal, pos);

    GetOwner()->SetPosition(positionLocal);

}
