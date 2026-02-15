#pragma once
#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Components/Camera/CameraComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Physics/Physics.h"


#include "Engine/Camera/CameraConstants.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/DarkGame/DarkActors/FightStage.h"

class Camera :public Actor
{
public:
    enum class EProjectionType :uint8_t
    {
        Perspective,
        Orthographic
    };
public:
    //引数付きコンストラクタ
    Camera(std::string actorName) :Actor(actorName)
    {
    }

    virtual ~Camera() = default;

    virtual void Initialize(const Transform& transform)override
    {
        mainCameraComponent = this->AddComponent<class TPSCameraComponent>("mainCamera");
        mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(45), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 1.1f, 100.0f);
    }

    virtual ViewConstants GetViewConstants(/*EProjectionType type = EProjectionType::Perspective*/) const
    {
        ViewConstants viewConstants;
        DirectX::XMFLOAT3 cameraPosition = GetPosition();
        viewConstants.cameraPosition = { cameraPosition.x,cameraPosition.y,cameraPosition.z,1.0f };
        viewConstants.view = mainCameraComponent->GetView();
        viewConstants.projection = mainCameraComponent->GetProjection();

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&mainCameraComponent->GetProjection());
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&mainCameraComponent->GetView());
        DirectX::XMStoreFloat4x4(&viewConstants.viewProjection, V * P);

        DirectX::XMStoreFloat4x4(&viewConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        DirectX::XMStoreFloat4x4(&viewConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        DirectX::XMStoreFloat4x4(&viewConstants.invView, DirectX::XMMatrixInverse(NULL, V));
        return viewConstants;
    }
protected:
    std::shared_ptr<CameraComponent> mainCameraComponent;
};

class DebugCamera :public Camera
{
public:
    //引数付きコンストラクタ
    DebugCamera(std::string actorName) :Camera(actorName) {}

    virtual ~DebugCamera() = default;
    void Initialize(const Transform& transform)override
    {
        debugCameraComponent = this->AddComponent<class DebugCameraComponent>("debugCamera");
    }

    virtual ViewConstants GetViewConstants() const override
    {
        ViewConstants viewConstants;
        DirectX::XMFLOAT3 cameraPosition = GetPosition();
        viewConstants.cameraPosition = { cameraPosition.x,cameraPosition.y,cameraPosition.z,1.0f };
        viewConstants.view = debugCameraComponent->GetView();
        viewConstants.projection = debugCameraComponent->GetProjection();

        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&debugCameraComponent->GetProjection());
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&debugCameraComponent->GetView());
        DirectX::XMStoreFloat4x4(&viewConstants.viewProjection, V * P);

        DirectX::XMStoreFloat4x4(&viewConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        DirectX::XMStoreFloat4x4(&viewConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        DirectX::XMStoreFloat4x4(&viewConstants.invView, DirectX::XMMatrixInverse(NULL, V));
        return viewConstants;
    }
private:
    std::shared_ptr<DebugCameraComponent> debugCameraComponent;
};

class MainCamera :public Camera
{
public:
    //引数付きコンストラクタ
    explicit MainCamera(const std::string& actorName) :Camera(actorName) {}
    virtual ~MainCamera() = default;

    void Initialize(const Transform& transform)override
    {
        Camera::Initialize(transform);
    };

    //更新処理
    void Update(float deltaTime)override;

    void Shake(float power = 0.02f, float time = 0.2f)
    {
        //mainCameraComponent->Shake(power, time);
    }
    void AddYaw(float v)
    {
        mainCameraComponent->yaw += v;
    }

    void AddPitch(float v)
    {
        mainCameraComponent->pitch = std::clamp(mainCameraComponent->pitch + v, -1.5f, 1.5f);
    }

    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI

#endif
    }

    DirectX::XMFLOAT3 CameraForwardXZ() const
    {
        auto V = mainCameraComponent->GetView();
        DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&V);

        // Viewの逆行列 = カメラのワールド行列
        DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, view);

        DirectX::XMFLOAT3 forward;
        DirectX::XMStoreFloat3(
            &forward,
            DirectX::XMVector3Normalize(invView.r[2]) // Z軸
        );

        // XZに投影
        forward.y = 0.0f;

        float len = sqrtf(forward.x * forward.x + forward.z * forward.z);
        if (len > 0.0001f)
        {
            forward.x /= len;
            forward.z /= len;
        }

        return forward;
    }

    DirectX::XMFLOAT3 CameraRightXZ() const
    {
        auto forward = CameraForwardXZ();

        return {
            forward.z,
            0.0f,
            -forward.x
        };
    }



private:
    DirectX::XMFLOAT3 offset = { 0.6f,11.4f,-15.4f };
    DirectX::XMFLOAT3 cameraMin = { -11.0f,0.0f,-8.0f };
    DirectX::XMFLOAT3 cameraMax = { 11.0f,0.0f,7.0f };

    float distanceX = 0.0f;
    float distanceY = 0.0f;
    float distanceZ = 0.0f;
    float elapsedTime = 0.0f;


    bool didShake = false;
};


