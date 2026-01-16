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

    const DirectX::XMFLOAT4X4& GetOrthographicProjection()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixOrthographicLH(15 * aspect, 15, nearZ, farZ));
        return projection;
    }

    const DirectX::XMFLOAT4X4& GetProjectionWithOrthographic()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixPerspectiveLH(15 * aspect, 15, nearZ, farZ));
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
            DirectX::XMConvertToRadians(80.0f)
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

        //// 衝突解決後のカメラ位置
        //XMVECTOR finalEye =
        //    ResolveCameraCollision(focus, idealEye);

        XMStoreFloat4x4(
            &view,
            XMMatrixLookAtLH(idealEye, focus, XMVectorSet(0, 1, 0, 0))
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
    }


private:
    float moveSpeed = 5.0f;
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