#pragma once
#include "Core/Actor.h"
#include "UI/Widgets/Widget.h"
#include "StageData.h"

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

    // 目標クリアタイムのUIを表示する
    void SetTargetTime(STAGE_NAME stage);

private:
    void UpdateTimerDigits(int totalSeconds);

private:
    std::vector<std::shared_ptr<UIImageComponent>> timerDigits;

    std::shared_ptr<UIImageComponent> timerFrameImage;
    std::shared_ptr<UIImageComponent> timerTargetImage;

    // 配置
    float spacing = 33.0f;
    DirectX::XMFLOAT2 numberSize = { 35.0f,49.0f };
    DirectX::XMFLOAT2 minuteSpacing = { -72.0f,53.0f };
    DirectX::XMFLOAT2 secondSpacing = { -75.0f,80.0f };

    DirectX::XMFLOAT2 targetTimeOffset = { 0.0f, 113.0f }; // 目標タイムのUIの位置オフセット
    DirectX::XMFLOAT2 targetTimeSize = { 404.0f, 352.0f }; // 目標タイムのUIの位置オフセット
    DirectX::XMFLOAT2 timerFrameUiPos={1774.0f,302.0f};
};
