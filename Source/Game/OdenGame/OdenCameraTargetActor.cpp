#include "pch.h"
#include "OdenCameraTargetActor.h"

void OdenCameraTargetActor::Initialize(const Transform& transform)
{
    easingComponent = AddComponent<CoreEasingComponent>("easingComponent");
}

void OdenCameraTargetActor::Update(float elapsedTime)
{
    DirectX::XMFLOAT3 position = MathHelper::Lerp(originPos, targetPos, easingValue);
    SetPosition(position);
}

void OdenCameraTargetActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button("PlayToTarget"))
    {
        PlayToTarget(moveTimer);
    }
    if (ImGui::Button("PlayToOrigin"))
    {
        PlayToOrigin(moveTimer);
    }
    ImGui::SliderFloat("moveTimer", &moveTimer, 0.0f, 6.0f);
#endif
}

void OdenCameraTargetActor::PlayToTarget(float moveTime)
{
    StartEasing(moveTime, 0.0f, 1.0f);
    if (onMoveFinished)
        onMoveFinished();

}

void OdenCameraTargetActor::PlayToOrigin(float moveTime)
{
    StartEasing(moveTime, 1.0f, 0.0f);
    if (onMoveFinished)
        onMoveFinished();
}

void OdenCameraTargetActor::StartEasing(float moveTime, float from, float to)
{
    TestEasingHandler handler;
    handler.AddEasing(TestEaseType::OutExp, from, to, moveTime);

    handler.SetCompletedFunction([this]()
        {
            //if (onMoveFinished)
            //    onMoveFinished();
        });

    PropertyAccessor<float> accessor;
    accessor.getter = [this]() { return easingValue; };
    accessor.setter = [this](float t) { easingValue = t; };

    easingComponent->StartHandler(handler, accessor);
}
