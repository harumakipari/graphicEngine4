#include "pch.h"
#include "CameraComponent.h"

#include "Core/Actor.h"

//// ビュー行列取得
//const DirectX::XMFLOAT4X4& CameraComponent::GetView()
//{
//#if 1
//    using namespace DirectX;
//    //using namespace DirectX;
//    //// ターゲット座標
//    //XMFLOAT3 targetPos{ 0,0,0 };
//    //if (!target.expired())
//    //{
//    //    targetPos = target.lock()
//    //        ->GetComponentWorldTransform()
//    //        .GetLocation();
//    //}
//
//    //XMVECTOR focus = XMLoadFloat3(&targetPos);
//
//    //// 回転（Yaw → Pitch）
//    //XMMATRIX rot =
//    //    XMMatrixRotationY(yaw) *
//    //    XMMatrixRotationX(pitch);
//
//    //// 後ろ方向に distance
//    //XMVECTOR offset =
//    //    XMVector3TransformCoord(
//    //        XMVectorSet(0, 0, -distance, 0),
//    //        rot
//    //    );
//
//    //XMVECTOR eye = focus + offset;
//
//    //XMMATRIX viewMat = XMMatrixLookAtLH(
//    //    eye,
//    //    focus,
//    //    XMVectorSet(0, 1, 0, 0)
//    //);
//
//    //XMStoreFloat4x4(&view, viewMat);
//    //return view;
//
//#if 0
//    // クォータニオンから回転行列
//    XMMATRIX rotationMatrix{};
//    XMVECTOR eye{};
//    //XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), rotationMatrix);
//    if (auto parent = attachParent_.lock())
//    {
//        rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&parent->GetRelativeRotation()));
//        DirectX::XMFLOAT3 e = parent->GetRelativeLocation();
//        //e = attachParent_.lock()->GetLocalPosition();
//        //rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&attachParent_.lock()->GetLocalRotation()));
//        //DirectX::XMFLOAT3 e = attachParent_.lock()->GetWorldPosition();
//        //e = attachParent_.lock()->GetLocalPosition();
//        eye = XMLoadFloat3(&e);
//    }
//    else
//    {
//        rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&GetRelativeRotation()));
//        DirectX::XMFLOAT3 pos = GetRelativeLocation();
//        eye = XMLoadFloat3(&pos);
//    }
//    // 前方向ベクトルを回転
//    XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rotationMatrix);
//    XMVECTOR up = /*XMVector3TransformNormal(*/XMVectorSet(0, 1, 0, 0)/*, rotationMatrix)*/;
//
//
//    XMVECTOR target = eye + forward;
//
//    if (customTarget)
//    {
//        target = XMLoadFloat3(&_target);
//    }
//
//    //カメラシェイク処理
//    if (handler.GetSequenceCount() > 0)
//    {
//        XMVECTOR right = XMVector3Cross(up, forward);
//
//        float x = ((rand() / static_cast<float>(RAND_MAX)) - 0.5f) * power;
//        float y = ((rand() / static_cast<float>(RAND_MAX)) - 0.5f) * power;
//        target += (right * x) + (up * y);
//    }
//
//    XMMATRIX ViewMatrix = XMMatrixLookAtLH(eye, target, up);
//
//    XMStoreFloat4x4(&view, ViewMatrix);
//    return view;
//#else
//    XMFLOAT3 position = owner_.lock()->GetPosition();
//    if (!lookAtTarget.expired())
//    {
//        XMFLOAT3 lookAtTargetPos = lookAtTarget.lock()->GetComponentLocation();
//        XMMATRIX ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&position),
//            XMLoadFloat3(&lookAtTargetPos),
//            XMVectorSet(0, 1, 0, 0));
//        XMStoreFloat4x4(&view, ViewMatrix);
//        return view;
//    }
//    XMFLOAT4 rotation = owner_.lock()->GetQuaternionRotation();
//    XMVECTOR Quaternion = XMLoadFloat4(&rotation);
//    XMVECTOR Forward = XMVector3TransformNormal(
//        XMVectorSet(0, 0, -1, 0), // Unityのforwardと反対 (-Z)
//        XMMatrixRotationQuaternion(Quaternion) // 回転を適用
//    );
//    XMVECTOR Focus = XMLoadFloat3(&position) + Forward;
//    XMVECTOR Eye = Focus + Forward * distance;
//    XMVECTOR Up = XMVector3TransformNormal(
//        XMVectorSet(0, 1, 0, 0),
//        XMMatrixRotationQuaternion(Quaternion)
//    );
//    XMMATRIX ViewMatrix = XMMatrixLookAtLH(Eye, Focus, Up);
//    XMStoreFloat4x4(&view, ViewMatrix);
//    return view;
//
//
//#endif // 0
//#else
//    using namespace DirectX;
//
//    XMMATRIX mat = XMLoadFloat4x4(&GetWorldTransform());
//
//    // カメラ位置と向きからビュー行列作成
//    XMVECTOR eyePos = XMVector3TransformCoord(XMVectorZero(), mat);
//    XMVECTOR lookDir = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), mat); // Z+方向
//    XMVECTOR lookAt = XMVectorAdd(eyePos, lookDir);
//    XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), mat);
//
//    XMStoreFloat4x4(&view, XMMatrixLookAtLH(eyePos, lookAt, up));
//    return view;
//#endif
//}
//const DirectX::XMFLOAT4X4& SimpleCameraComponent::GetView()
//{
//    using namespace DirectX;
//    // ターゲット座標
//    XMFLOAT3 targetPos{ 0,0,0 };
//    if (!target.expired())
//    {
//        targetPos = target.lock()
//            ->GetComponentWorldTransform()
//            .GetLocation();
//    }
//
//    XMVECTOR focus = XMLoadFloat3(&targetPos);
//
//    // 回転（Yaw → Pitch）
//    XMMATRIX rot =
//        XMMatrixRotationY(yaw) *
//        XMMatrixRotationX(pitch);
//
//    // 後ろ方向に distance
//    XMVECTOR offset =
//        XMVector3TransformCoord(
//            XMVectorSet(0, 0, -distance, 0),
//            rot
//        );
//
//    XMVECTOR eye = focus + offset;
//
//    XMMATRIX viewMat = XMMatrixLookAtLH(
//        eye,
//        focus,
//        XMVectorSet(0, 1, 0, 0)
//    );
//
//    XMStoreFloat4x4(&view, viewMat);
//    return view;
//}
//
//
//const DirectX::XMFLOAT4X4& SimpleCameraComponent::GetProjection()
//{
//    using namespace DirectX;
//    XMStoreFloat4x4(
//        &projection,
//        XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ)
//    );
//    return projection;
//}
//

void DebugCameraComponent::HandleKeyboardInput(float deltaTime)
{
    using namespace DirectX;
    DirectX::XMFLOAT4 rotaion = GetComponentRotation();
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
    DirectX::XMVECTOR forward = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), rotationMatrix);
    //DirectX::XMVECTOR right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), rotationMatrix);
    //DirectX::XMVECTOR up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), rotationMatrix);
    DirectX::XMVECTOR right = rotationMatrix.r[0];
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

    DirectX::XMVECTOR move = DirectX::XMVectorZero();
    if (float wheelDelta = ImGui::GetIO().MouseWheel)
    {
        move += forward * wheelDelta * 30.0f;


        //distance -= wheelDelta;
        //distance = std::clamp(distance, 0.1f,100.0f);
    }

    if (InputSystem::GetInputState("W")) { move += forward; }
    if (InputSystem::GetInputState("S")) { move -= forward; }
    if (InputSystem::GetInputState("D")) { move += right; }
    if (InputSystem::GetInputState("A")) { move -= right; }
    //
    if (InputSystem::GetInputState("E")) { move += up; }
    if (InputSystem::GetInputState("Q")) { move -= up; }

    if (InputSystem::GetInputState("Shift")) { move = DirectX::XMVectorScale(move, 2.5f); }

    move = DirectX::XMVectorScale(move, moveSpeed * deltaTime);

    //DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&positionLocal);
    //pos += move;
    //DirectX::XMStoreFloat3(&positionLocal, pos);
    DirectX::XMFLOAT3 position = GetComponentLocation();
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    pos += move;
    DirectX::XMFLOAT3 positionLocal{};
    DirectX::XMStoreFloat3(&positionLocal, pos);

    GetOwner()->SetPosition(positionLocal);
    //SetWorldLocationDirect(positionLocal);
    ///
    ///
        //マウスホイールで距離変更有効
        //if (isEnableWheel)
    {
        //// シーンビュー上でのみホイール操作を受け付ける
        //float left, top, right, bottom;
        //Graphics::GetScreenRect(left, top, right, bottom);

        //// マウスがシーンビュー上にあるか
        //if (ImGui::IsMouseHoveringRect(ImVec2(left, top), ImVec2(right, bottom), false))
        {
            //ホイールで距離変更
            //if (float wheelDelta = ImGui::GetIO().MouseWheel)
            //{
            //    move += forward*wheelDelta;


            //    //distance -= wheelDelta;
            //    //distance = std::clamp(distance, 0.1f,100.0f);
            //}
        }
    }

}
