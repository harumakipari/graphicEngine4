#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

// C++ 標準ライブラリ
#include <string>

// 他ライブラリ
#include <DirectXMath.h>

#include "Engine/Camera/BookmarkCamera.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif

// プロジェクトの他のヘッダ
#include "Components/Base/SceneComponent.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "Engine/Camera/CameraConstants.h"
#include "Engine/Input/InputSystem.h"

class CameraComponent :public SceneComponent
{
public:
    struct CameraBookmark
    {
        DirectX::XMFLOAT3 position;

        float yaw;
        float pitch;
        float fov;
    };

    CameraBookmark bookmark;
    bool hasBookmark = false;

    CameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner) {}

    // パースペクティブ設定
    void SetPerspective(const float fovY, const float aspect, const float nearZ, const float farZ)
    {
        this->fovY = fovY;
        this->aspect = aspect;
        this->nearZ = nearZ;
        this->farZ = farZ;
    }

    void SaveBookmark()
    {
        bookmark.position = GetComponentLocation();
        bookmark.yaw = yaw;
        bookmark.pitch = pitch;
        bookmark.fov = fovY;
        
        hasBookmark = true;
    }

    void LoadBookmark()
    {
        if (!hasBookmark) return;

        GetOwner()->SetPosition(bookmark.position);
        yaw = bookmark.yaw;
        pitch = bookmark.pitch;
        fovY = bookmark.fov;
        UpdateRotationFromYawPitch();
    }

    const DirectX::XMFLOAT4X4& GetView();

    const DirectX::XMFLOAT4X4& GetProjection()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
        return projection;
    }

    CameraState GetState() const
    {
        CameraState s;
        s.position = GetComponentLocation();
        s.rotation = { pitch, yaw, 0 };
        s.fov = fovY;
        return s;
    }

    void SetState(const CameraState& s)
    {
        SetWorldLocationDirect(s.position);
        pitch = s.rotation.x;
        yaw = s.rotation.y;
        fovY = s.fov;
    }

    bool useLookTarget = false;
    DirectX::XMFLOAT3 lookTarget{};

    ViewConstants GetViewConstants();

    // CameraActorのrotationを更新する
    void UpdateRotationFromYawPitch()
    {
        using namespace DirectX;

        XMVECTOR qYaw = XMQuaternionRotationAxis(
            XMVectorSet(0, 1, 0, 0),
            yaw);

        XMVECTOR qPitch = XMQuaternionRotationAxis(
            XMVectorSet(1, 0, 0, 0),
            pitch);

        XMVECTOR q = XMQuaternionNormalize(
            XMQuaternionMultiply(qPitch, qYaw));

        XMFLOAT4 rot;
        XMStoreFloat4(&rot, q);

        GetOwner()->SetQuaternionRotation(rot);
    }

    void AddYaw(const float v)
    {
        yaw += v;
        UpdateRotationFromYawPitch();
    }

    void AddPitch(const float v)
    {
        pitch += v;

        pitch = std::clamp(
            pitch,
            -DirectX::XM_PIDIV2 + 0.01f,
            DirectX::XM_PIDIV2 - 0.01f);

        UpdateRotationFromYawPitch();
    }

    void SetYawAndPitch(const float yaw, const float pitch)
    {
        this->yaw = yaw;
        this->pitch = pitch;
        UpdateRotationFromYawPitch();
    }

    float GetYaw() const { return yaw; }

    float GetPitch() const { return pitch; }

    float GetFov()const { return fovY; }

    void SetFov(float fov)
    {
        SetPerspective(fovY, Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
    }
protected:
    float fovY = DirectX::XMConvertToRadians(35.0f); 
    float aspect = 1280.f / 720.f;
    float nearZ = 0.1f;
    float farZ = 1000.f;
    float yaw = 0.0f;
    float pitch = DirectX::XMConvertToRadians(-12.0f);

    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};
};


// 三人称視点のカメラ
class TPSCameraComponent : public CameraComponent
{
public:
    TPSCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :CameraComponent(name, owner) {}

    void Tick(const float deltaTime) override
    {
        easingComponent.Tick(deltaTime);

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
            ImGui::DragFloat3(U8("注視点のオフセット"), &targetOffset.x, 0.1f);
            ImGui::DragFloat3(U8("カメラの位置のオフセット"), &cameraOffset.x, 0.1f);
            // ===== yaw / pitch / fov を degree 表示 =====
            float yawDeg = DirectX::XMConvertToDegrees(yaw);
            float pitchDeg = DirectX::XMConvertToDegrees(pitch);
            float fovDeg = DirectX::XMConvertToDegrees(fovY);
            if (ImGui::DragFloat("FOV (deg)", &fovDeg, 0.5f, 10.0f, 120.0f))
            {
                fovY = DirectX::XMConvertToRadians(fovDeg);
            }
            if (ImGui::DragFloat("yaw (deg)", &yawDeg, 0.5f))
            {
                yaw = DirectX::XMConvertToRadians(yawDeg);
            }
            if (ImGui::DragFloat("pitch (deg)", &pitchDeg, 0.5f))
            {
                pitch = DirectX::XMConvertToRadians(pitchDeg);
            }
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);
            ImGui::DragFloat("farZ", &farZ, 0.1f);
            ImGui::SliderFloat("distance", &distance, 0.01f, 100.0f);
            ImGui::Checkbox("Camera Lock", &cameraLock);
            ImGui::SliderFloat(U8("追従するスピード"), &autoFollowStrength, 0.05f, 10.0f);

            ImGui::TreePop();
        }
#endif
    }

    //const DirectX::XMFLOAT4X4& GetView() override;


    // 自動随従する
    void AutoFollow(const DirectX::XMFLOAT3& moveDir, const DirectX::XMFLOAT2& rightStick, float deltaTime);

    // カメラの距離をイージングで変化させる
    void PlayDistance(const float from, const float to, const float time)
    {
        distanceFrom = from;
        distanceTo = to;

        TestEasingHandler handler;
        handler.AddEasing(TestEaseType::OutExp, 0.0f, 1.0f, time);

        PropertyAccessor<float> accessor;
        accessor.getter = [this]() { return distanceEasingValue; };
        accessor.setter = [this](const float t)
            {
                distanceEasingValue = t;
                distance = std::lerp(distanceFrom, distanceTo, t);
            };

        easingComponent.StartHandler(handler, accessor);
    }

    float distance = 4.5f;
    DirectX::XMFLOAT3 targetOffset = { 0.0f, 1.5f, 0.0f }; // 注視点のオフセット。キャラクターの頭あたりを注視するために Y を 1.5f くらいにしている
    DirectX::XMFLOAT3 cameraOffset = { 0.0f, 0.0f, 0.0f }; // カメラの位置を微調整するためのオフセット




private:
    DirectX::XMVECTOR ResolveCameraCollision(
        DirectX::FXMVECTOR focus,
        DirectX::FXMVECTOR idealEye
    );


    static float WrapAngle(float a)
    {
        using namespace DirectX;

        while (a > XM_PI)  a -= XM_2PI;
        while (a < -XM_PI) a += XM_2PI;
        return a;
    }
private:
    // 自動追従のためのパラメータ
    float autoFollowStrength = 2.0f; // 追従の強さ。大きいほど素早く追従する
    float autoFollowDeadZone = 0.15f;
    float autoFollowDelayTimer = 0.0f;
    float autoFollowDelay = 1.2f; // カメラの方向を変えたら、一定時間追従しないようにするための遅延時間
    float autoFollowDegree = 25.0f; // 追従する角度の閾値（degree）

    float distanceEasingValue = 0.0f;
    float distanceFrom = 0.0f;
    float distanceTo = 0.0f;

    EasingRunner easingComponent;
    bool cameraLock = false;
};


class DebugCameraComponent :public CameraComponent
{
public:
    DebugCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :CameraComponent(name, owner) {}

    void Tick(const float deltaTime)override
    {
        if (!useDebug) return;
        HandleKeyboardInput(deltaTime);
        HandleMouseInput(deltaTime);
        if (InputSystem::GetInputState("F5",InputStateMask::Release))
        {
            SaveBookmark();
        }

        if (InputSystem::GetInputState("F6", InputStateMask::Release))
        {
            LoadBookmark();
        }
    }

    void SetIsUseDebug(const bool useDebug) { this->useDebug = useDebug; }


private:

    void HandleKeyboardInput(float deltaTime);
    void HandleMouseInput(float deltaTime)
    {
#if 1
        if (InputSystem::GetInputState("MouseRight"))
        {
            int dx, dy;
            InputSystem::GetMouseDelta(dx, dy);

            AddYaw(dx * rotateSpeed);
            AddPitch(dy * rotateSpeed);

            //yaw += dx * rotateSpeed;
            //pitch += dy * rotateSpeed;

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
        GetOwner()->SetQuaternionRotation(rot);
        return;
#endif // 0


        if (InputSystem::GetInputState("MouseRight"))
        {
            int dx, dy;
            InputSystem::GetMouseDelta(dx, dy);


            float yawDelta = dx * rotateSpeed;
            float pitchDelta = dy * rotateSpeed ;

            using namespace DirectX;

            //XMFLOAT4 rot = GetComponentRotation();
            XMFLOAT4 rot = GetOwner()->GetQuaternionRotation();
            XMVECTOR q = XMLoadFloat4(&rot);

            XMVECTOR qYaw = XMQuaternionRotationAxis(
                XMVectorSet(0, 1, 0, 0),
                yawDelta
            );

            XMVECTOR right = XMVector3Rotate(
                XMVectorSet(1, 0, 0, 0),
                q
            );

            XMVECTOR qPitch = XMQuaternionRotationAxis(
                right,
                pitchDelta
            );

            q = XMQuaternionMultiply(q, qYaw);
            q = XMQuaternionMultiply(q, qPitch);

            q = XMQuaternionNormalize(q);

            XMStoreFloat4(&rot, q);
            GetOwner()->SetQuaternionRotation(rot);
        }
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  camera").c_str()))
        {
            ImGui::DragFloat("moveSpeed", &moveSpeed, 0.1f);
            ImGui::DragFloat("rotateSpeed", &rotateSpeed, 0.1f);

            // ===== yaw / pitch / fov を degree 表示 =====
            float yawDeg = DirectX::XMConvertToDegrees(yaw);
            float pitchDeg = DirectX::XMConvertToDegrees(pitch);
            float fovDeg = DirectX::XMConvertToDegrees(fovY);
            if (ImGui::DragFloat("FOV (deg)", &fovDeg, 0.5f, 10.0f, 120.0f))
            {
                fovY = DirectX::XMConvertToRadians(fovDeg);
            }
            if (ImGui::DragFloat("yaw (deg)", &yawDeg, 0.5f))
            {
                yaw = DirectX::XMConvertToRadians(yawDeg);
            }
            if (ImGui::DragFloat("pitch (deg)", &pitchDeg, 0.5f))
            {
                pitch = DirectX::XMConvertToRadians(pitchDeg);
            }
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);

            ImGui::TreePop();
        }
#endif
    }


private:
    bool useDebug = false;
    float moveSpeed = 5.0f;
    float rotateSpeed = 0.001f;

};

class CinematicCameraComponent:public CameraComponent
{
public:
    CinematicCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :CameraComponent(name, owner) {}


    void Tick(const float deltaTime)override
    {
        if (!useCinematic) return;
        HandleKeyboardInput(deltaTime);
        HandleMouseInput(deltaTime);
    }

    void SetIsUseCinematic(const bool useCinematic) { this->useCinematic = useCinematic; }

private:
    float moveSpeed = 5.0f;
    float rotateSpeed = 0.001f;
    bool useCinematic = false;
    

    void HandleKeyboardInput(float deltaTime);
    void HandleMouseInput(float deltaTime)
    {
#if 1
        if (InputSystem::GetInputState("MouseRight"))
        {
            int dx, dy;
            InputSystem::GetMouseDelta(dx, dy);

            AddYaw(dx * rotateSpeed);
            AddPitch(dy * rotateSpeed);

            //yaw += dx * rotateSpeed;
            //pitch += dy * rotateSpeed;

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
        GetOwner()->SetQuaternionRotation(rot);
        return;
#endif // 0


        if (InputSystem::GetInputState("MouseRight"))
        {
            int dx, dy;
            InputSystem::GetMouseDelta(dx, dy);


            float yawDelta = dx * rotateSpeed;
            float pitchDelta = dy * rotateSpeed;

            using namespace DirectX;

            //XMFLOAT4 rot = GetComponentRotation();
            XMFLOAT4 rot = GetOwner()->GetQuaternionRotation();
            XMVECTOR q = XMLoadFloat4(&rot);

            XMVECTOR qYaw = XMQuaternionRotationAxis(
                XMVectorSet(0, 1, 0, 0),
                yawDelta
            );

            XMVECTOR right = XMVector3Rotate(
                XMVectorSet(1, 0, 0, 0),
                q
            );

            XMVECTOR qPitch = XMQuaternionRotationAxis(
                right,
                pitchDelta
            );

            q = XMQuaternionMultiply(q, qYaw);
            q = XMQuaternionMultiply(q, qPitch);

            q = XMQuaternionNormalize(q);

            XMStoreFloat4(&rot, q);
            GetOwner()->SetQuaternionRotation(rot);
        }
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  camera").c_str()))
        {
            ImGui::DragFloat("moveSpeed", &moveSpeed, 0.1f);
            ImGui::DragFloat("rotateSpeed", &rotateSpeed, 0.1f);

            // ===== yaw / pitch / fov を degree 表示 =====
            float yawDeg = DirectX::XMConvertToDegrees(yaw);
            float pitchDeg = DirectX::XMConvertToDegrees(pitch);
            float fovDeg = DirectX::XMConvertToDegrees(fovY);
            if (ImGui::DragFloat("FOV (deg)", &fovDeg, 0.5f, 10.0f, 120.0f))
            {
                fovY = DirectX::XMConvertToRadians(fovDeg);
            }
            if (ImGui::DragFloat("yaw (deg)", &yawDeg, 0.5f))
            {
                yaw = DirectX::XMConvertToRadians(yawDeg);
            }
            if (ImGui::DragFloat("pitch (deg)", &pitchDeg, 0.5f))
            {
                pitch = DirectX::XMConvertToRadians(pitchDeg);
            }
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);

            ImGui::TreePop();
        }
#endif
    }

    void SaveBookmark(int i)
    {
        if (i >= bookmarks.size())
            bookmarks.resize(i + 1);

        bookmarks[i] = {
            GetComponentLocation(),
            yaw,
            pitch,
            fovY
        };
    }

    std::vector<CameraBookmark> bookmarks;
};


#endif //CAMERA_COMPONENT_H