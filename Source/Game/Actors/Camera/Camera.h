#pragma once
#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Components/Camera/CameraComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Physics/Physics.h"


#include "Engine/Camera/CameraConstants.h"
#include "Engine/Camera/TPSCameraController.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/DarkGame/DarkActors/DarkStage.h"

class Camera :public Actor
{
public:
    //引数付きコンストラクタ
    explicit Camera(const std::string& actorName) :Actor(actorName)
    {
    }

    virtual ~Camera() = default;

    virtual void Initialize(const Transform& transform)override;



    virtual ViewConstants GetViewConstants() const
    {
        return mainCameraComponent->GetViewConstants();
    }
protected:
    std::shared_ptr<CameraComponent> mainCameraComponent;
};

class DebugCamera :public Camera
{
public:
    //引数付きコンストラクタ
    DebugCamera(const std::string& actorName) :Camera(actorName) {}

    virtual ~DebugCamera() = default;
    void Initialize(const Transform& transform)override
    {
        mainCameraComponent = this->AddComponent<class DebugCameraComponent>("debugCamera");
    }

    virtual ViewConstants GetViewConstants() const override
    {
        return mainCameraComponent->GetViewConstants();

        //ViewConstants viewConstants;
        //DirectX::XMFLOAT3 cameraPosition = GetPosition();
        //viewConstants.cameraPosition = { cameraPosition.x,cameraPosition.y,cameraPosition.z,1.0f };
        //viewConstants.view = debugCameraComponent->GetView();
        //viewConstants.projection = debugCameraComponent->GetProjection();

        //DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&debugCameraComponent->GetProjection());
        //DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&debugCameraComponent->GetView());
        //DirectX::XMStoreFloat4x4(&viewConstants.viewProjection, V * P);

        //DirectX::XMStoreFloat4x4(&viewConstants.invProjection, DirectX::XMMatrixInverse(NULL, P));
        //DirectX::XMStoreFloat4x4(&viewConstants.invViewProjection, DirectX::XMMatrixInverse(NULL, V * P));

        //DirectX::XMStoreFloat4x4(&viewConstants.invView, DirectX::XMMatrixInverse(NULL, V));

        //viewConstants.cameraClipDistance.x = debugCameraComponent->GetNearClipDistance();
        //viewConstants.cameraClipDistance.y = debugCameraComponent->GetFarClipDistance();
        //viewConstants.cameraClipDistance.z = debugCameraComponent->GetNearClipDistance() * debugCameraComponent->GetFarClipDistance();
        //viewConstants.cameraClipDistance.w = debugCameraComponent->GetFarClipDistance() - debugCameraComponent->GetNearClipDistance();

        //return viewConstants;
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
        tpsController.camera =
            static_cast<TPSCameraComponent*>(mainCameraComponent.get());
    };

    void SetTarget(const std::shared_ptr<SceneComponent>& target)
    {
        tpsController.target = target;
    }

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
        float yaw = mainCameraComponent->yaw;

        return {
            sinf(yaw),
            0.0f,
            cosf(yaw)
        };
    }

    DirectX::XMFLOAT3 CameraRightXZ() const
    {
        float yaw = mainCameraComponent->yaw;

        return {
            cosf(yaw),
            0.0f,
            -sinf(yaw)
        };
    }



private:

    TPSCameraController tpsController;

    bool didShake = false;
};


