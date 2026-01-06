#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>

#include "Core/Actor.h"
#include "Core/ActorManager.h"

#include "Components/Camera/CameraComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Physics/Physics.h"


#include "Engine/Camera/CameraConstants.h"
#include "Game/Actors/Stage/ElasticBuilding.h"
#include "Game/Actors/Stage/FightStage.h"

class Camera :public Actor
{
public:
    //引数付きコンストラクタ
    Camera(std::string actorName) :Actor(actorName) 
    {
    }

    virtual ~Camera() = default;

    virtual void Initialize(const Transform& transform)override
    {
        mainCameraComponent = this->AddComponent<class TPSCameraComponent>("mainCamera");
        mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(45), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
    }

    virtual ViewConstants GetViewConstants() const
    {
        ViewConstants viewConstants;
        DirectX::XMFLOAT3 cameraPosition = GetPosition();
        viewConstants.cameraPosition={ cameraPosition.x,cameraPosition.y,cameraPosition.z,1.0f };
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
    MainCamera(std::string actorName) :Camera(actorName) {}

    virtual ~MainCamera() = default;
    std::shared_ptr<SphereComponent> sphereComponent;
    void Initialize(const Transform& transform)override
    {
        Camera::Initialize(transform);
        // 当たり判定のコンポーネントを追加
        //sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", "springArm");
        //sphereComponent = this->AddComponent<class SphereComponent>("sphereComponent", mainCameraComponent->name());
        //sphereComponent->SetRadius(0.2f);
        ////sphereComponent->SetMass(40.0f);
        //sphereComponent->SetLayer(CollisionLayer::Camera);
        //sphereComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Trigger);
        //sphereComponent->SetResponseToLayer(CollisionLayer::Camera, CollisionComponent::CollisionResponse::None);
        //sphereComponent->Initialize();
        //sphereComponent->SetIsVisibleDebugBox(false);
        //sphereComponent->SetIsVisibleDebugShape(false);
        //sphereComponent->DisableCollision();
    };

    //更新処理
    void Update(float deltaTime)override
    {
        using namespace DirectX;
        switch (state)
        {
        case MainCamera::State::Normal:
        {
            //mainCameraComponent->customTarget = true;

            DirectX::XMVECTOR vOld = XMLoadFloat3(&oldTarget);
            DirectX::XMVECTOR vNew = XMLoadFloat3(&target);

            float speed = 2.0f;
            float t = std::clamp(deltaTime * speed, 0.0f, 1.0f);

            XMVECTOR vInterp = XMVectorLerp(vOld, vNew, t);
            XMFLOAT3 interpTarget;
            DirectX::XMStoreFloat3(&interpTarget, vInterp);

            DirectX::XMFLOAT3 clampedTarget;
            clampedTarget.x = std::clamp(interpTarget.x, cameraMin.x, cameraMax.x);
            clampedTarget.y = interpTarget.y;
            clampedTarget.z = std::clamp(interpTarget.z, cameraMin.z, cameraMax.z);

            // 補間済み目標位置を使って、eye にオフセットを適用
            DirectX::XMFLOAT3 eye;
            eye.x = clampedTarget.x + offset.x;
            eye.y = clampedTarget.y + offset.y;
            eye.z = clampedTarget.z + offset.z;
            SetPosition(eye);

            oldTarget = clampedTarget;

            //DirectX::XMFLOAT3 eye = GetPosition();
            DirectX::XMVECTOR eyeVec = DirectX::XMLoadFloat3(&eye);
            DirectX::XMFLOAT3 tar = target;
            tar.y += 0.5f;// player の腰当たり
            DirectX::XMVECTOR targetVec = DirectX::XMLoadFloat3(&tar);
            DirectX::XMVECTOR dirVec = DirectX::XMVectorSubtract(targetVec, eyeVec);
            DirectX::XMVECTOR dirNormalized = DirectX::XMVector3Normalize(dirVec);
            // 前フレームのターゲット
            preTarget = tar;
            //mainCameraComponent->_target = clampedTarget;
            // lerp 先の position を保存
            afterTarget = clampedTarget;
            afterEye = eye;
            HitResultWithActor result;
            DirectX::XMFLOAT3 origin = GetPosition();
            DirectX::XMFLOAT3 direction;
            direction.x = mainCameraComponent->GetView()._31;
            direction.y = -mainCameraComponent->GetView()._32;
            direction.z = mainCameraComponent->GetView()._33;
            DirectX::XMVECTOR distanceVec = DirectX::XMVector3Length(dirVec);
            float distance;
            DirectX::XMVECTOR DirVec = DirectX::XMLoadFloat3(&direction);
            DirVec = DirectX::XMVector3Normalize(DirVec);
            DirectX::XMStoreFloat3(&direction, DirVec);
            DirectX::XMStoreFloat(&distance, distanceVec);
            distance = 100.0f;
            //if (PhysicsTest::Instance().SphereCast(origin, direction, distance, 0.4f, result, CollisionHelper::ToBit(CollisionLayer::Camera),     // myLayer
            //    CollisionHelper::ToBit(CollisionLayer::Building)))   // wantHitRayer)

                // レイキャストテスト
            //HitResult hit;
            //if (Physics::Instance().SphereCast(
            //    DirectX::XMFLOAT3(origin.x, origin.y + 1.5f, origin.z),
            //    direction,
            //    FLT_MAX,
            //    0.1f, hit))
            //{
            //    Graphics::GetShapeRenderer()->DrawSphere(hit.position, 0.1f, { 1, 0, 0, 1 });
            //}



            if (Physics::Instance().SphereCast(origin, direction, FLT_MAX, 0.1f, result))   // wantHitRayer)
            {
                if (auto build = dynamic_cast<FightStage*>(result.actor))
                {
                    Graphics::GetShapeRenderer()->DrawSphere(result.hitPoint, 0.1f, { 0, 0, 0, 1 });
                }
                if (auto build = dynamic_cast<ElasticBuilding*>(result.actor))
                {
                    build->skeltalMeshComponent->SetIsVisible(false);
                }
            }
            else
            {
            }
        }
        break;
        case MainCamera::State::BossTarget:
        {

        }
        break;
        case MainCamera::State::Lerp:
        {
            elapsedTime += deltaTime;
            float lerpTime = 1.5f;
            float t = std::clamp(elapsedTime / lerpTime, 0.0f, 1.0f);
            DirectX::XMVECTOR PreTargetVec = DirectX::XMLoadFloat3(&preTarget);
            DirectX::XMVECTOR PreEyeVec = DirectX::XMLoadFloat3(&preEye);

            // 最初の　target　はプレイヤーの初期位置
            DirectX::XMVECTOR AftTargetVec = DirectX::XMLoadFloat3(&afterTarget);
            DirectX::XMVECTOR AftEyeVec = DirectX::XMLoadFloat3(&afterEye);
            DirectX::XMVECTOR NowTar = XMVectorLerp(PreTargetVec, AftTargetVec, t);
            DirectX::XMVECTOR NowEye = XMVectorLerp(PreEyeVec, AftEyeVec, t);
            DirectX::XMFLOAT3 nowTarget, nowEye;
            DirectX::XMStoreFloat3(&nowTarget, NowTar);
            DirectX::XMStoreFloat3(&nowEye, NowEye);
            //mainCameraComponent->customTarget = true;
            //mainCameraComponent->_target = nowTarget;
            SetPosition(nowEye);
            if (t >= 1.0f)
            {// lerp し終わったら
                state = State::Normal;
            }
        }
        break;
        default:
            break;
        }
    }

    void Shake(float power = 0.02f, float time = 0.2f)
    {
        //mainCameraComponent->Shake(power, time);
    }

    void SetOldTarget(const DirectX::XMFLOAT3& oldTarget)
    {
        this->oldTarget = oldTarget;
    }

    void DrawImGuiDetails()override
    {
#ifdef USE_IMGUI

#endif
    }
    // ボスに注視点を合わせるかどうか
    void IsTargetBoss(bool isTargetBoss)
    {
        this->isTargetBoss = isTargetBoss;
        elapsedTime = 0.0f;
        if (isTargetBoss)
        {
            state = State::BossTarget;
        }
    }

    void SetTarget(DirectX::XMFLOAT3 target)
    {
        this->target = target;
    }

    void OnFinishFirstPerf()
    {
        isFinishFirstPerf = true;
    }
private:
    DirectX::XMFLOAT3 target = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 offset = { 0.6f,11.4f,-15.4f };
    DirectX::XMFLOAT3 oldTarget = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 cameraMin = { -11.0f,0.0f,-8.0f };
    DirectX::XMFLOAT3 cameraMax = { 11.0f,0.0f,7.0f };
    bool isTargetBoss = false;

    float distanceX = 0.0f;
    float distanceY = 0.0f;
    float distanceZ = 0.0f;
    //float distanceX = 0.0f;
    //float distanceY = -1.7f;
    //float distanceZ = 5.5f;
    // 最初の演出後の focus の位置
    DirectX::XMFLOAT3 preTarget = { 0.0f,0.0f,0.0f };
    float elapsedTime = 0.0f;

    bool isFinishFirstPerf = false;

    // 最初の演出後のカメラの位置
    DirectX::XMFLOAT3 preEye = { 0.0f,0.0f,0.0f };

    // lerp 先の
    DirectX::XMFLOAT3 afterTarget = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 afterEye = { 0.0f,0.0f,0.0f };

    enum class State
    {
        Normal,
        BossTarget,
        Lerp,
    };

    State state = State::Normal;

    bool didShake = false;
};


#endif //CAMERA_H