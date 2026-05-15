#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"

// 　
// 　タイマーのUI表示
//
class GameTimerUiActor :public Actor
{
public:
    explicit GameTimerUiActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

    void DrawImGuiDetails() override;

private:
    void UpdateTimerDigits(int totalSeconds);

private:
    std::vector<std::shared_ptr<UIImageComponent>> timerDigits;

    std::shared_ptr<UIImageComponent> timerFrameImage;
    // 配置
    float spacing = 33.0f;
    DirectX::XMFLOAT2 numberSize = { 35.0f,49.0f };
    DirectX::XMFLOAT2 minuteSpacing = { -24.0f,0.0f };
    DirectX::XMFLOAT2 secondSpacing = { -27.0f,19.0f };
};
