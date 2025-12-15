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

#if 0
class SpringArmComponent :public SceneComponent
{
public:
    SpringArmComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner)
    {
        armLength = 5.0f;
        targetOffset = { 0.0f,1.0f,0.0f };
    }

    void Tick(float deltaTime)override
    {
        using namespace DirectX;

        if (auto parent = attachParent_.lock())
        {
            // 1. 親のワールド位置・回転取得
            //XMFLOAT3 parentWorldPos = parent->GetWorldPosition();
            XMFLOAT3 parentWorldPos = parent->GetRelativeLocation();
            XMVECTOR parentPosVec = XMLoadFloat3(&parentWorldPos);
            XMVECTOR parentRotQuat = XMLoadFloat4(&parent->GetRelativeRotation());
            //XMVECTOR parentRotQuat = XMLoadFloat4(&parent->GetLocalRotation());


            // 
            XMVECTOR localRot = DirectX::XMLoadFloat4(&GetRelativeRotation());
            //XMVECTOR localRot = XMQuaternionRotationRollPitchYaw(
            //    XMConvertToRadians(angleLocal.x),
            //    XMConvertToRadians(angleLocal.y),
            //    XMConvertToRadians(angleLocal.z));

            // 親の回転と合成
            XMVECTOR finalRot = XMQuaternionMultiply(localRot, parentRotQuat);

            XMMATRIX rotMatrix = XMMatrixRotationQuaternion(finalRot);



            // 親の角度を確認して配置するカメラ
            XMVECTOR forward = XMVector3TransformNormal(
                XMVectorSet(0, 0, 1, 0),
                XMMatrixRotationQuaternion(finalRot)
            );

            forward = XMVectorSet(0, 0, 1, 0);  // こっちがキャラクターの回転を無視して後ろに配置するカメラ

            XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), XMMatrixRotationQuaternion(finalRot));


            XMVECTOR backward = XMVectorScale(forward, -armLength);
            XMVECTOR upward = XMVectorScale(up, targetOffset.y);

            // 3. 位置計算
            XMVECTOR camPos = parentPosVec + backward + upward;
            XMFLOAT3 camPosF3;
            XMStoreFloat3(&camPosF3, camPos);

            // 4. ワールド位置・回転反映
            //SetWorldPosition(camPosF3);
            //rotationLocal = parent->GetLocalRotation();


            //SetWorldLocationDirect(camPosF3);
            SetRelativeLocationDirect(camPosF3);
            //positionLocal = camPosF3;
            DirectX::XMFLOAT4 rotationLocal;
            XMStoreFloat4(&rotationLocal, finalRot);    // 親の角度を確認して配置するカメラ
            SetWorldRotationDirect(rotationLocal);
            rotationLocal = { 0.0f,0.0f,0.0f,1.0f };    // こっちがキャラクターの回転を無視して後ろに配置するカメラ
            SetWorldRotationDirect(rotationLocal);
        }
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  armSpring").c_str()))
        {
            //ImGui::DragFloat3("Position", &positionLocal.x, 0.1f);
            ImGui::DragFloat("armLength", &armLength, 0.1f);
            ImGui::TreePop();
        }
#endif
    }


private:
    float armLength;
    bool enableLag;
    float lagSpeed;
    DirectX::XMFLOAT3 targetOffset;
};

#endif // 0

class SimpleCameraComponent :public SceneComponent
{
public:
    // ===== カメラパラメータ =====
    float yaw = 0.0f;     // 左右（Y軸回転）
    float pitch = -20.0f;   // 上下（X軸回転）
    float distance = 10.0f; // ターゲットからの距離

    float fovY = DirectX::XMConvertToRadians(60.0f);
    float aspect = 1280.0f / 720.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    SimpleCameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) : SceneComponent(name, owner) {};

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + " simple camera").c_str()))
        {
            ImGui::DragFloat("fovY", &fovY, 0.1f);
            ImGui::SliderFloat("nearZ", &nearZ, 0.01f, 100.0f);
            ImGui::DragFloat("farZ", &farZ, 0.1f);
            ImGui::SliderFloat("distance", &distance, 0.0f, 100.0f);
            ImGui::DragFloat("yaw", &yaw, 0.1f);
            ImGui::DragFloat("pitch", &pitch, 0.1f);
            ImGui::TreePop();
        }
#endif
    }



    // 注視ターゲット
    std::weak_ptr<SceneComponent> target;

public:
    const DirectX::XMFLOAT4X4& GetView();
    const DirectX::XMFLOAT4X4& GetProjection();

private:
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};
};



class CameraComponent :public SceneComponent
{
public:
    CameraComponent(const std::string& name, const std::shared_ptr<Actor>& owner) : SceneComponent(name, owner), power(0)
    {
        // デフォルトパラメータ
        fovY = DirectX::XMConvertToRadians(45); // 45度
        aspectRatio = Graphics::GetScreenWidth() / Graphics::GetScreenHeight();
        nearZ = 0.1f;
        farZ = 1000.0f;
    }

    void Tick(float deltaTime)override
    {
#if 1
        Transform transform = owner_.lock()->GetRootComponent()->GetComponentWorldTransform();

        using namespace DirectX;
        //追従
        if (!followTarget.expired())
        {
            Transform followTransform = followTarget.lock()->GetComponentWorldTransform();

            XMFLOAT3 targetPosition = followTransform.GetLocation();
            XMFLOAT3 pos = owner_.lock()->GetRootComponent()->GetComponentWorldTransform().GetTranslation();
            //XMFLOAT3 targetPos = Vector3::ToXMFLOAT3(Vector3(followTarget->WorldPosition()) + Vector3(0, 0, -50));
            XMFLOAT3 forward = owner_.lock()->GetForward();
            DirectX::XMVECTOR TargetPos = XMLoadFloat3(&targetPosition) + DirectX::XMLoadFloat3(&forward) * distance;
            DirectX::XMFLOAT3 setPos;
            XMStoreFloat3(&setPos, XMVectorLerp(XMLoadFloat3(&pos), TargetPos, deltaTime * followSpeed));
            //transform->SetPosition(setPos);
            XMFLOAT3 translatePos = { setPos.x - transform.GetLocation().x,setPos.y - transform.GetLocation().y, setPos.z - transform.GetLocation().z };
            pos.x += translatePos.x;
            pos.y += translatePos.y;
            pos.z += translatePos.z;

            owner_.lock()->SetPosition(pos);
        }

        //注視
        if (!lookAtTarget.expired())
        {
            Transform lookAtTargetTransform = lookAtTarget.lock()->GetComponentWorldTransform();
            XMFLOAT4 rotation;
            XMFLOAT3 position = transform.GetLocation();
            XMFLOAT3 targetPosition = lookAtTargetTransform.GetLocation();
            XMFLOAT4 myRotation = transform.GetRotation();
            XMVECTOR Forward = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&position), XMLoadFloat3(&targetPosition)));
            XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMVECTOR Right = XMVector3Normalize(XMVector3Cross(Up, Forward));
            Up = XMVector3Cross(Forward, Right);
            XMMATRIX Rotation = XMMatrixIdentity();
            Rotation.r[0] = Right; Rotation.r[1] = Up; Rotation.r[2] = Forward;
            XMVECTOR Quaternion = XMQuaternionRotationMatrix(Rotation);
            XMStoreFloat4(&rotation,
                XMQuaternionSlerp(XMLoadFloat4(&myRotation), Quaternion, deltaTime * lookAtSpeed));
            owner_.lock()->SetQuaternionRotation(rotation);
        }

        //if (GetAsyncKeyState('F') & 0x8000) {
        //    Shake();
        //}

        //handler.Update(power, deltaTime);

#endif // 0
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
            ImGui::SliderFloat("distance", &distance, 0.01f, 100.0f);
            ImGui::DragFloat("minDistance", &minDistance, 0.1f);
            ImGui::DragFloat("maxDistance", &maxDistance, 0.1f);
            ImGui::DragFloat("yaw", &yaw, 0.1f);
            ImGui::DragFloat("pitch", &pitch, 0.1f);

            ImGui::TreePop();
        }
#endif
    }


    void LookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
    {
        using namespace DirectX;
        XMVECTOR eyeVec = XMLoadFloat3(&eye);
        XMVECTOR targetVec = XMLoadFloat3(&target);
        XMVECTOR upVec = XMLoadFloat3(&up);
        XMMATRIX viewMat = XMMatrixLookAtLH(eyeVec, targetVec, upVec);
        XMStoreFloat4x4(&view, viewMat);
    }


    // パースペクティブ設定
    void SetPerspective(float fovY, float aspect, float nearZ, float farZ)
    {
        this->fovY = fovY;
        this->aspectRatio = aspect;
        this->nearZ = nearZ;
        this->farZ = farZ;
    }

    // ビュー行列取得
    const DirectX::XMFLOAT4X4& GetView();

    //　プロジェクション行列取得
    const DirectX::XMFLOAT4X4& GetProjection()
    {
        using namespace DirectX;
        XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearZ, farZ));
        return projection;
    }

    //カメラシェイク
    void Shake(float power = 0.02f, float time = 0.2f)
    {
#if 0
        handler.Clear();
        handler.SetEasing(EaseType::Linear, power, 0.f, time);

#endif // 0
    }

    //フォーカスを手動設定できるようにする用
    bool customTarget = false;
    DirectX::XMFLOAT3 _target{};

public:
    float yaw = 0.0f;     // 左右（Y軸回転）
    float pitch = -20.0f;   // 上下（X軸回転）
    std::weak_ptr<SceneComponent> target;

    /** @brief 追従ターゲット。*/
    std::weak_ptr<SceneComponent> followTarget;
    /** @brief 注視ターゲット。*/
    std::weak_ptr<SceneComponent> lookAtTarget;
    /** @brief ターゲットからの距離。*/
    float distance = 10.0f;
    /** @brief 最小距離。*/
    float minDistance = 0.1f;
    /** @brief 最大距離。*/
    float maxDistance = 1000.f;
    /** @brief 追従補間速度。*/
    float followSpeed = 5.f;
    /** @brief 注視補間速度。*/
    float lookAtSpeed = 5.f;
    /** @brief 視野角（度）。*/
    float fovY = 60.0f;
    /** @brief アスペクト比（幅/高さ）。*/
    float aspectRatio = 1280.0f / 720.0f;
    /** @brief 近クリップ面。*/
    float nearZ = 0.1f;
    /** @brief 遠クリップ面。*/
    float farZ = 1000.0f;

protected:
    /** @brief マウスホイールでのズーム有効/無効。*/
    bool isEnableWheel = true;

private:

    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 projection{};

    EasingHandler handler;
    float power;
};





class DebugCameraComponent :public CameraComponent
{
public:
    DebugCameraComponent(const std::string& name, std::shared_ptr<Actor> owner) :CameraComponent(name, owner) {}

    void Tick(float deltaTime)override
    {
        HandleKeyboardInput(deltaTime);
        HandleMouseInput(deltaTime);
#ifdef USE_IMGUI
        //マウスホイールで距離変更有効
        if (isEnableWheel)
        {
            //// シーンビュー上でのみホイール操作を受け付ける
            //float left, top, right, bottom;
            //Graphics::GetScreenRect(left, top, right, bottom);

            //// マウスがシーンビュー上にあるか
            //if (ImGui::IsMouseHoveringRect(ImVec2(left, top), ImVec2(right, bottom), false))
            {
                //ホイールで距離変更
                if (float wheelDelta = ImGui::GetIO().MouseWheel)
                {
                    distance -= wheelDelta;
                    distance = std::clamp(distance, minDistance, maxDistance);
                }
            }
        }
#endif // USE_IMGUI
    }


private:
    float moveSpeed = 5.0f;
    float rotateSpeed = 1.0f;

    float yaw = 0.0f;
    float pitch = 0.0f;

    void HandleKeyboardInput(float deltaTime);

    void HandleMouseInput(float deltaTime)
    {
        if (InputSystem::GetInputState("MouseRight"))
        {
            int deltaX, deltaY;
            InputSystem::GetMouseDelta(deltaX, deltaY);

#if 1
            //DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationLocal));
            DirectX::XMFLOAT4 rotaion = GetComponentRotation();
            DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotaion));
            yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
            pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
            pitch = std::clamp(pitch, DirectX::XMConvertToRadians(-89.0f), DirectX::XMConvertToRadians(89.0f));
            DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
            DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), yaw);
            DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
            //DirectX::XMVECTOR Q = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
            DirectX::XMFLOAT4 r = GetComponentRotation();
            DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&r), Q));
            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotationLocal), Q));
#else
            DirectX::XMVECTOR rot;
            {
                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
                pitch = static_cast<float>(deltaY) * rotateSpeed * deltaTime;
                DirectX::XMVECTOR quatPitch = DirectX::XMQuaternionRotationAxis(R.r[0], pitch);
                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), quatPitch));
            }
            {
                DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(rot);
                yaw = static_cast<float>(deltaX) * rotateSpeed * deltaTime;
                DirectX::XMVECTOR quatYaw = DirectX::XMQuaternionRotationAxis(R.r[1], yaw);
                rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(rot, quatYaw));
            }
#endif // 0

            //DirectX::XMVECTOR rot = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quatYaw, quatPitch));
            //rot= DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&rotation), rot));
            //R = DirectX::XMMatrixRotationQuaternion(rot);
            ////R.r[2] = {};
            //rot = DirectX::XMQuaternionRotationMatrix(R);
            DirectX::XMFLOAT4 rotationLocal{};
            DirectX::XMStoreFloat4(&rotationLocal, rot);
            SetWorldRotationDirect(rotationLocal);
        }
    }
};


#endif //CAMERA_COMPONENT_H