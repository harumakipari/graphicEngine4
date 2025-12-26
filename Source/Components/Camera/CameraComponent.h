#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

// C++ 標準ライブラリ
#include <string>

// 他ライブラリ
#include <DirectXMath.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

// プロジェクトの他のヘッダ
#include "Components/Base/SceneComponent.h"
#include "Core/Actor.h"
#include "Engine/Input/InputSystem.h"
#include "Utils/EasingHandler.h"

class CameraComponent :public SceneComponent
{
public:
    CameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner) {}
    // パースペクティブ設定
    void SetPerspective(float fovY, float aspect, float nearZ, float farZ)
    {
        this->fovY = fovY;
        this->aspect = aspect;
        this->nearZ = nearZ;
        this->farZ = farZ;
    }
    virtual const DirectX::XMFLOAT4X4& GetView() = 0;

    const DirectX::XMFLOAT4X4& GetProjection()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
        return projection;
    }

protected:
    float fovY = DirectX::XMConvertToRadians(60.0f);
    float aspect = 1280.f / 720.f;
    float nearZ = 0.1f;
    float farZ = 1000.f;
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};
};

// Transformを更新しない三人称視点のカメラ
class TPSCameraComponent : public CameraComponent
{
public:
    TPSCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :CameraComponent(name, owner) {}

    void Tick(float deltaTime) override
    {
        pitch = std::clamp(
            pitch,
            DirectX::XMConvertToRadians(-60.0f),
            DirectX::XMConvertToRadians(20.0f)
        );
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  camera").c_str()))
        {
            ImGui::DragFloat3("targetOffset", &targetOffset.x, 0.1f);
            ImGui::DragFloat("fovY", &fovY, 0.1f);
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);
            ImGui::DragFloat("farZ", &farZ, 0.1f);
            ImGui::SliderFloat("distance", &distance, 0.01f, 100.0f);
            // ===== yaw / pitch を degree 表示 =====
            float yawDeg = DirectX::XMConvertToDegrees(yaw);
            float pitchDeg = DirectX::XMConvertToDegrees(pitch);

            if (ImGui::DragFloat("yaw (deg)", &yawDeg, 0.5f))
            {
                yaw = DirectX::XMConvertToRadians(yawDeg);
            }

            if (ImGui::DragFloat("pitch (deg)", &pitchDeg, 0.5f))
            {
                pitch = DirectX::XMConvertToRadians(pitchDeg);
            }

            ImGui::TreePop();
        }
#endif
    }



    const DirectX::XMFLOAT4X4& GetView() {
        using namespace DirectX;

        XMFLOAT3 basePos{ 0,0,0 };
        if (!target.expired())
            basePos = target.lock()->GetComponentWorldTransform().GetLocation();

        XMVECTOR focus = XMLoadFloat3(&basePos) +
            XMVectorSet(targetOffset.x, targetOffset.y, targetOffset.z, 0);

        // Yaw → Pitch
        XMMATRIX yawRot = XMMatrixRotationY(yaw);

        XMVECTOR right = XMVector3TransformNormal(
            XMVectorSet(1, 0, 0, 0),
            yawRot
        );

        XMMATRIX pitchRot = XMMatrixRotationAxis(right, pitch);
        XMMATRIX rot = pitchRot * yawRot;

        // 後方に距離分下がる
        XMVECTOR offset = XMVector3TransformNormal(
            XMVectorSet(0, 0, -distance, 0),
            rot
        );

        // 理想のカメラ位置
        XMVECTOR idealEye = focus + offset;

        // 衝突解決後のカメラ位置
        XMVECTOR finalEye =
            ResolveCameraCollision(focus, idealEye);

        XMStoreFloat4x4(
            &view,
            XMMatrixLookAtLH(finalEye, focus, XMVectorSet(0, 1, 0, 0))
        );

        return view;
    }

    float yaw = 0.0f;
    float pitch = DirectX::XMConvertToRadians(-12.0f);
    float distance = 4.5f;
    DirectX::XMFLOAT3 targetOffset = { 0.0f, 1.5f, 0.0f };
    std::weak_ptr<SceneComponent> target;

private:
    DirectX::XMVECTOR ResolveCameraCollision(
        DirectX::FXMVECTOR focus,
        DirectX::FXMVECTOR idealEye
    )
    {
        using namespace DirectX;

        XMFLOAT3 f, e;
        XMStoreFloat3(&f, focus);
        XMStoreFloat3(&e, idealEye);

        XMFLOAT3 dir = {
            e.x - f.x,
            e.y - f.y,
            e.z - f.z
        };

        float len = sqrtf(dir.x * dir.x + dir.z * dir.z + dir.y * dir.y);
        if (len < 0.001f)
            return idealEye;

        dir.x /= len; dir.y /= len; dir.z /= len;

        HitResult hit;
        if (Physics::Instance().RayCast(
            f, dir, len,
            hit,
            CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
        {
            // 少し手前に出す
            XMVECTOR h = XMLoadFloat3(&hit.position);
            return h - XMVectorScale(XMLoadFloat3(&dir), 0.2f);
        }

        return idealEye;
    }
};

//class CameraComponent :public SceneComponent
//{
//public:
//    CameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) : SceneComponent(name, owner), power(0)
//    {
//        // デフォルトパラメータ
//        fovY = DirectX::XMConvertToRadians(45); // 45度
//        aspectRatio = Graphics::GetScreenWidth() / Graphics::GetScreenHeight();
//        nearZ = 0.1f;
//        farZ = 1000.0f;
//    }
//
//    void Tick(float deltaTime)override
//    {
//#ifdef USE_IMGUI
//        //マウスホイールで距離変更有効
//        if (isEnableWheel)
//        {
//            //// シーンビュー上でのみホイール操作を受け付ける
//            //float left, top, right, bottom;
//            //Graphics::GetScreenRect(left, top, right, bottom);
//
//            //// マウスがシーンビュー上にあるか
//            //if (ImGui::IsMouseHoveringRect(ImVec2(left, top), ImVec2(right, bottom), false))
//            {
//                //ホイールで距離変更
//                if (float wheelDelta = ImGui::GetIO().MouseWheel)
//                {
//                    distance -= wheelDelta;
//                    distance = std::clamp(distance, minDistance, maxDistance);
//                }
//            }
//        }
//#endif // USE_IMGUI
//
//#if 1
//        Transform transform = owner_.lock()->GetRootComponent()->GetComponentWorldTransform();
//
//        using namespace DirectX;
//        //追従
//        if (!followTarget.expired())
//        {
//            Transform followTransform = followTarget.lock()->GetComponentWorldTransform();
//
//            XMFLOAT3 targetPosition = followTransform.GetLocation();
//            XMFLOAT3 pos = owner_.lock()->GetRootComponent()->GetComponentWorldTransform().GetTranslation();
//            //XMFLOAT3 targetPos = Vector3::ToXMFLOAT3(Vector3(followTarget->WorldPosition()) + Vector3(0, 0, -50));
//            XMFLOAT3 forward = owner_.lock()->GetForward();
//            DirectX::XMVECTOR TargetPos = XMLoadFloat3(&targetPosition) + DirectX::XMLoadFloat3(&forward) * distance;
//            DirectX::XMFLOAT3 setPos;
//            XMStoreFloat3(&setPos, XMVectorLerp(XMLoadFloat3(&pos), TargetPos, deltaTime * followSpeed));
//            //transform->SetPosition(setPos);
//            XMFLOAT3 translatePos = { setPos.x - transform.GetLocation().x,setPos.y - transform.GetLocation().y, setPos.z - transform.GetLocation().z };
//            pos.x += translatePos.x;
//            pos.y += translatePos.y;
//            pos.z += translatePos.z;
//
//            owner_.lock()->SetPosition(pos);
//        }
//
//        //注視
//        if (!lookAtTarget.expired())
//        {
//            Transform lookAtTargetTransform = lookAtTarget.lock()->GetComponentWorldTransform();
//            XMFLOAT4 rotation;
//            XMFLOAT3 position = transform.GetLocation();
//            XMFLOAT3 targetPosition = lookAtTargetTransform.GetLocation();
//            XMFLOAT4 myRotation = transform.GetRotation();
//            XMVECTOR Forward = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&position), XMLoadFloat3(&targetPosition)));
//            XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//            XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Up, Forward));
//            Up = XMVector3Cross(Forward, Right);
//            XMMATRIX Rotation = XMMatrixIdentity();
//            Rotation.r[0] = Right; Rotation.r[1] = Up; Rotation.r[2] = Forward;
//            XMVECTOR Quaternion = XMQuaternionRotationMatrix(Rotation);
//            XMStoreFloat4(&rotation,
//                XMQuaternionSlerp(XMLoadFloat4(&myRotation), Quaternion, deltaTime * lookAtSpeed));
//            owner_.lock()->SetQuaternionRotation(rotation);
//        }
//
//        //if (GetAsyncKeyState('F') & 0x8000) {
//        //    Shake();
//        //}
//
//        //handler.Update(power, deltaTime);
//
//#endif // 0
//    }
//
//    virtual void DrawImGuiInspector() override
//    {
//#ifdef USE_IMGUI
//
//        SceneComponent::DrawImGuiInspector();
//        if (ImGui::TreeNode((name_ + "  camera").c_str()))
//        {
//            //ImGui::DragFloat3("Position", &positionLocal.x, 0.1f);
//            ImGui::DragFloat("fovY", &fovY, 0.1f);
//            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);
//            ImGui::DragFloat("farZ", &farZ, 0.1f);
//            ImGui::SliderFloat("distance", &distance, 0.01f, 100.0f);
//            ImGui::DragFloat("minDistance", &minDistance, 0.1f);
//            ImGui::DragFloat("maxDistance", &maxDistance, 0.1f);
//            ImGui::DragFloat("yaw", &yaw, 0.1f);
//            ImGui::DragFloat("pitch", &pitch, 0.1f);
//
//            ImGui::TreePop();
//        }
//#endif
//    }
//
//
//    void LookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
//    {
//        using namespace DirectX;
//        XMVECTOR eyeVec = XMLoadFloat3(&eye);
//        XMVECTOR targetVec = XMLoadFloat3(&target);
//        XMVECTOR upVec = XMLoadFloat3(&up);
//        XMMATRIX viewMat = XMMatrixLookAtLH(eyeVec, targetVec, upVec);
//        XMStoreFloat4x4(&view, viewMat);
//    }
//
//
//    // パースペクティブ設定
//    void SetPerspective(float fovY, float aspect, float nearZ, float farZ)
//    {
//        this->fovY = fovY;
//        this->aspectRatio = aspect;
//        this->nearZ = nearZ;
//        this->farZ = farZ;
//    }
//
//    // ビュー行列取得
//    const DirectX::XMFLOAT4X4& GetView();
//
//    //　プロジェクション行列取得
//    const DirectX::XMFLOAT4X4& GetProjection()
//    {
//        using namespace DirectX;
//        XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ));
//        return projection;
//    }
//
//    //カメラシェイク
//    void Shake(float power = 0.02f, float time = 0.2f)
//    {
//#if 0
//        handler.Clear();
//        handler.SetEasing(EaseType::Linear, power, 0.f, time);
//
//#endif // 0
//    }
//
//    //フォーカスを手動設定できるようにする用
//    bool customTarget = false;
//    DirectX::XMFLOAT3 _target{};
//
//public:
//    float yaw = 0.0f;     // 左右（Y軸回転）
//    float pitch = -20.0f;   // 上下（X軸回転）
//    std::weak_ptr<SceneComponent> target;
//
//    /** @brief 追従ターゲット。*/
//    std::weak_ptr<SceneComponent> followTarget;
//    /** @brief 注視ターゲット。*/
//    std::weak_ptr<SceneComponent> lookAtTarget;
//    /** @brief ターゲットからの距離。*/
//    float distance = 10.0f;
//    /** @brief 最小距離。*/
//    float minDistance = 0.1f;
//    /** @brief 最大距離。*/
//    float maxDistance = 1000.f;
//    /** @brief 追従補間速度。*/
//    float followSpeed = 5.f;
//    /** @brief 注視補間速度。*/
//    float lookAtSpeed = 5.f;
//    /** @brief 視野角（度）。*/
//    float fovY = 60.0f;
//    /** @brief アスペクト比（幅/高さ）。*/
//    float aspectRatio = 1280.0f / 720.0f;
//    /** @brief 近クリップ面。*/
//    float nearZ = 0.1f;
//    /** @brief 遠クリップ面。*/
//    float farZ = 1000.0f;
//
//protected:
//    /** @brief マウスホイールでのズーム有効/無効。*/
//    bool isEnableWheel = true;
//
//private:
//
//    DirectX::XMFLOAT4X4 view{};
//    DirectX::XMFLOAT4X4 projection{};
//
//    EasingHandler handler;
//    float power;
//};
//
//



class DebugCameraComponent :public CameraComponent
{
public:
    DebugCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :CameraComponent(name, owner) {}
    const DirectX::XMFLOAT4X4& GetView() override
    {
        using namespace DirectX;

        XMFLOAT3 pos = GetComponentLocation();
        XMFLOAT4 rot = GetComponentRotation();

        XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&rot));
        XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);

        XMMATRIX viewMtx = XMMatrixInverse(nullptr, R * T);
        XMStoreFloat4x4(&view, viewMtx);
        return view;
    }

    void Tick(float deltaTime)override
    {
        HandleKeyboardInput(deltaTime);
        HandleMouseInput(deltaTime);
#ifdef USE_IMGUI
        //マウスホイールで距離変更有効
        //if (isEnableWheel)
        //{
        //    //// シーンビュー上でのみホイール操作を受け付ける
        //    //float left, top, right, bottom;
        //    //Graphics::GetScreenRect(left, top, right, bottom);

        //    //// マウスがシーンビュー上にあるか
        //    //if (ImGui::IsMouseHoveringRect(ImVec2(left, top), ImVec2(right, bottom), false))
        //    {
        //        //ホイールで距離変更
        //        if (float wheelDelta = ImGui::GetIO().MouseWheel)
        //        {
        //            distance -= wheelDelta;
        //            distance = std::clamp(distance, minDistance, maxDistance);
        //        }
        //    }
        //}
#endif // USE_IMGUI
    }


private:
    float moveSpeed = 5.0f;
    //float rotateSpeed = 1.0f;
    float rotateSpeed = 0.002f;

    float yaw = 0.0f;
    float pitch = 0.0f;

    void HandleKeyboardInput(float deltaTime);
    void HandleMouseInput(float deltaTime)
    {
        if (InputSystem::GetInputState("MouseRight"))
        {
            int dx, dy;
            InputSystem::GetMouseDelta(dx, dy);

            yaw += dx * rotateSpeed;
            pitch += dy * rotateSpeed;

            // 上下向きすぎ防止（重要）
            pitch = std::clamp(pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);
        }
        using namespace DirectX;

        // ワールドY軸Yaw
        XMVECTOR qYaw = XMQuaternionRotationAxis(
            XMVectorSet(0, 1, 0, 0),
            yaw
        );

        // ローカルX軸Pitch
        XMVECTOR qPitch = XMQuaternionRotationAxis(
            XMVectorSet(1, 0, 0, 0),
            pitch
        );

        // 合成順序：Yaw → Pitch
        XMVECTOR q = XMQuaternionNormalize(
            XMQuaternionMultiply(qPitch, qYaw)
        );

        XMFLOAT4 rot;
        XMStoreFloat4(&rot, q);
        SetWorldRotationDirect(rot);

        return;

        if (InputSystem::GetInputState("MouseRight"))
        {
            int deltaX, deltaY;
            InputSystem::GetMouseDelta(deltaX, deltaY);

            float yawDelta = deltaX * rotateSpeed;
            float pitchDelta = deltaY * rotateSpeed;

            using namespace DirectX;

            XMFLOAT4 rot = GetComponentRotation();
            XMVECTOR q = XMLoadFloat4(&rot);

            // ワールドY軸でYaw
            XMVECTOR qYaw = XMQuaternionRotationAxis(
                XMVectorSet(0, 1, 0, 0),
                yawDelta
            );

            // ローカルX軸でPitch
            XMVECTOR right = XMVector3Rotate(
                XMVectorSet(1, 0, 0, 0),
                q
            );
            XMVECTOR qPitch = XMQuaternionRotationAxis(
                right,
                pitchDelta
            );

            // 合成（順番超重要）
            q = XMQuaternionNormalize(
                XMQuaternionMultiply(qPitch, q)
            );
            q = XMQuaternionNormalize(
                XMQuaternionMultiply(qYaw, q)
            );

            XMStoreFloat4(&rot, q);
            SetWorldRotationDirect(rot);
            //// 横移動 → Yaw（ワールドY）
            //yaw += deltaX * rotateSpeed;

            //// 縦移動 → Pitch（ローカルX）
            //pitch -= deltaY * rotateSpeed; // ← マイナス重要

            //pitch = std::clamp(pitch,
            //    DirectX::XMConvertToRadians(-80.f),
            //    DirectX::XMConvertToRadians(80.f));
//#if 1
//            //DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationLocal));
//            DirectX::XMFLOAT4 rotaion = GetComponentRotation();
//            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
//            yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
//            pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
//            pitch = std::clamp(pitch, DirectX::XMConvertToRadians(-89.0f), DirectX::XMConvertToRadians(89.0f));
//            DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
//            DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), yaw);
//            DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
//            //DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
//            DirectX::XMFLOAT4 r = GetComponentRotation();
//            DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&r), Q));
//            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotationLocal), Q));
//#else
//            DirectX::XMVECTOR rot;
//            {
//                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
//                pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
//                DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
//                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
//            }
//            {
//                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot);
//                yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
//                DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(R.r[1], yaw);
//                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(rot, quatYaw));
//            }
//#endif // 0
//
//            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
//            //rot= DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), rot));
//            //R = DirectX::XMMatrixRotationQuaternion(rot);
//            ////R.r[2] = {};
//            //rot = DirectX::XMQuaternionRotationMatrix(R);
//            DirectX::XMFLOAT4 rotationLocal{};
//            DirectX::XMStoreFloat4(&rotationLocal, rot);
//            SetWorldRotationDirect(rotationLocal);
        }
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  camera").c_str()))
        {
            //ImGui::DragFloat3("Position", &positionLocal.x, 0.1f);
            ImGui::DragFloat("fovY", &fovY, 0.1f);
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);
            ImGui::DragFloat("farZ", &farZ, 0.1f);
            //ImGui::SliderFloat("distance", &distance, 0.01f, 100.0f);
            //ImGui::DragFloat("minDistance", &minDistance, 0.1f);
            //ImGui::DragFloat("maxDistance", &maxDistance, 0.1f);
            //ImGui::DragFloat("yaw", &yaw, 0.1f);
            //ImGui::DragFloat("pitch", &pitch, 0.1f);

            ImGui::TreePop();
        }
#endif
    }

};


#endif //CAMERA_COMPONENT_H