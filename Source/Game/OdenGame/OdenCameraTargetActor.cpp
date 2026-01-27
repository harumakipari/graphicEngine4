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
    if (ImGui::Button("MoveCameraTarget"))
    {
        Play(moveTimer);
    }

    ImGui::SliderFloat("moveTimer", &moveTimer, 0.0f, 6.0f);
#endif
}

// ターゲットの移動開始
void OdenCameraTargetActor::Play(float moveTime)
{
    TestEasingHandler handler;
    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        moveTime
    );

    handler.SetCompletedFunction([this]()
        {
            SetPosition(targetPos);
            easingValue = 1.0f;
        });
    PropertyAccessor<float> accessor;

    accessor.getter = [this]() { return easingValue; };
    accessor.setter = [this](float t)
        {
            easingValue = t;
        };

    easingComponent->StartHandler(handler, accessor);
}

