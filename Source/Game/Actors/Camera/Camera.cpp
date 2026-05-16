#include "pch.h"
#include "Camera.h"

#include "Components/Controller/ControllerComponent.h"
#include "Engine/Scene/SceneBase.h"
#include "Game/ScissorsGame/BobbinActor.h"
#include "Game/ScissorsGame/RabbitBossEnemy.h"
#include "Game/ScissorsGame/ScissorsPlayer1.h"
#include "Physics/CollisionFunction.h"

void Camera::Initialize(const Transform& transform)
{
    mainCameraComponent = this->AddComponent<class TPSCameraComponent>("mainCamera");
    mainCameraComponent->SetPerspective(DirectX::XMConvertToRadians(35), Graphics::GetScreenWidth() / Graphics::GetScreenHeight(), 0.1f, 1000.0f);
}

void MainCamera::Update(float deltaTime)
{
    // プレイヤー移動方向
    XMFLOAT3 moveDir = {};

    if (auto target = tpsController.target.lock())
    {
        auto actor = target->GetOwner();

        if (auto movement = actor->GetComponent<CharacterMovementComponent>())
        {
            moveDir = movement->GetVelocity();
        }
    }

    // 右スティック
    XMFLOAT2 rightStick = InputSystem::GetRightStick();

    // カメラ回転
    mainCameraComponent->AddYaw(rightStick.x * deltaTime * 2.0f);
    mainCameraComponent->AddPitch(-rightStick.y * deltaTime * 2.0f);

    //const float limit = DirectX::XMConvertToRadians(80.0f);

    //mainCameraComponent->pitch =
    //    std::clamp(
    //        mainCameraComponent->pitch,
    //        -limit,
    //        limit
    //    );

    // Controller更新
    tpsController.Update(deltaTime);

}


void FixedCamera::Update(float deltaTime)
{
    XMFLOAT3 pos = GetPosition();
    auto boss = GetOwnerScene()->GetActorManager()->GetActorOfType<RabbitBossEnemyActor>();

    bool shouldFade = false;

    uint32_t mask = CollisionHelper::ToBit(CollisionLayer::Boss);

    // Player判定
    auto player = GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsPlayer1>();
    if (player)
    {
        HitResultWithActor hit;
        XMFLOAT3 playerPos = player->GetPosition();

        if (CollisionFunction::SphereRayCast(pos, playerPos, hit, 0.2f, mask))
        {
            shouldFade = true;
        }
    }

    // Bobbin判定
    auto bobbin = GetOwnerScene()->GetActorManager()->GetActorOfType<BobbinActor>();
    if (bobbin)
    {
        HitResultWithActor hit;
        XMFLOAT3 bobbinPos = bobbin->GetPosition();

        if (CollisionFunction::SphereRayCast(pos, bobbinPos, hit, 0.2f, mask))
        {
            shouldFade = true;
        }
    }


    // 最後に一回だけ反映
    if (boss)
    {
        if (boss->GetStateMachine()->GetStateName() == "Win")
        {// 勝利時は半透明にしない
            shouldFade = false;
        }

        boss->SetRenderOpacity(shouldFade ? 0.5f : 1.0f);
    }

    // Controller更新
    tpsController.Update(deltaTime);
}

void TitleCamera::Update(float deltaTime)
{
    easingYawRunner->Tick(deltaTime);
    easingPitchRunner->Tick(deltaTime);
    // Controller更新
    tpsController.Update(deltaTime);

    mainCameraComponent->SetPitch(DirectX::XMConvertToRadians(currentPitch));
    mainCameraComponent->SetYaw(DirectX::XMConvertToRadians(currentYaw));

}

void TitleCamera::Play(float interval)
{
    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            startPitch,
            endPitch,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                currentPitch = endPitch;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return currentPitch; };
        accessor.setter = [this](float t)
            {
                currentPitch = t;
            };

        easingPitchRunner->StartHandler(handler, accessor);
    }


    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            startYaw,
            endYaw,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                currentYaw = endYaw;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return currentYaw; };
        accessor.setter = [this](float t)
            {
                currentYaw = t;
            };

        easingYawRunner->StartHandler(handler, accessor);
    }
}

void TitleCamera::PlayReverse(float interval)
{
    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            endPitch,
            startPitch,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                currentPitch = startPitch;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return currentPitch; };
        accessor.setter = [this](float t)
            {
                currentPitch = t;
            };

        easingPitchRunner->StartHandler(handler, accessor);
    }


    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            endYaw,
            startYaw,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                currentYaw = startYaw;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return currentYaw; };
        accessor.setter = [this](float t)
            {
                currentYaw = t;
            };

        easingYawRunner->StartHandler(handler, accessor);
    }
}