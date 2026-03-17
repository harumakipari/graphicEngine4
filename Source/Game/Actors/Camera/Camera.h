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

    CameraComponent* GetCameraComponent() const { return mainCameraComponent.get(); }

    ViewConstants GetViewConstants() const
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

};
class CinemaCamera :public Camera
{
public:
    //引数付きコンストラクタ
    CinemaCamera(const std::string& actorName) :Camera(actorName) {}

    virtual ~CinemaCamera() = default;
    void Initialize(const Transform& transform)override
    {
        mainCameraComponent = this->AddComponent<CinematicCameraComponent>("cinemaCamera");
    }

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
    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI

#endif
    }

    DirectX::XMFLOAT3 CameraForwardXZ() const
    {
        float yaw = mainCameraComponent->GetYaw();

        return {
            sinf(yaw),
            0.0f,
            cosf(yaw)
        };
    }

    DirectX::XMFLOAT3 CameraRightXZ() const
    {
        float yaw = mainCameraComponent->GetYaw();

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


