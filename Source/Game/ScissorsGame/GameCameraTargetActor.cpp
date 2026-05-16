#include "pch.h"
#include "GameCameraTargetActor.h"

#include "RabbitBossEnemy.h"
#include "Engine/Scene/Scene.h"

void GameCameraTargetActor::Initialize(const Transform& transform)
{
    easingRunner = std::make_unique<EasingRunner>();
    startPosition = transform.GetLocation();
    easingValue = 0.0f;
}

void GameCameraTargetActor::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);
    currentPosition = MathHelper::Lerp(startPosition, endPosition, easingValue);
    SetPosition(currentPosition);

}

void GameCameraTargetActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("ターゲットをボスへ")))
    {
        Play(2.0f);
    }
#endif
}

// 
void GameCameraTargetActor::Play(float interval)
{
    easingValue = 0.0f;
    startPosition = GetPosition();
    if (auto boss = GetOwnerScene()->GetActorManager()->GetActorOfType<RabbitBossEnemyActor>())
    {
        endPosition = boss->GetPosition();
    }
    endPosition.y = 0.5f; // 少し上にずらす

    // position の easing
    {
        TestEasingHandler handler;

        handler.AddWait(0.0f);

        handler.AddEasing(
            TestEaseType::OutExp,
            0.0f,
            1.0f,
            interval
        );

        handler.SetCompletedFunction([this]()
            {
                easingValue = 1.0f;
            });
        PropertyAccessor<float> accessor;

        accessor.getter = [this]() { return easingValue; };
        accessor.setter = [this](float t)
            {
                easingValue = t;
            };

        easingRunner->StartHandler(handler, accessor);
    }
}


