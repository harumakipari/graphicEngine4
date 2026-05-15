#include "pch.h"

#include "GameTimerUiActor.h"

#include "ScissorsGameManager.h"
#include "ScissorsPlayer1.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

void GameTimerUiActor::Initialize(const Transform& transform)
{
    auto uiManager = GetOwnerScene()->GetUIManager();

    // MM:SS の4桁
    for (int i = 0; i < 4; i++)
    {
        auto digit = std::make_shared<UIImageComponent>(
            "./Data/Textures/ScissorsUI/numberWhite.png",
            "TimerDigit"
        );

        digit->SetSize({ 45, 60 });
        digit->SetPivot({ 0.5f, 0.5f });
        digit->SetColor(XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f });
        digit->zOrder = 2;

        uiManager->Add(digit);

        timerDigits.push_back(digit);
    }

    // コロン :
    timerFrameImage = std::make_shared<UIImageComponent>(
        "./Data/Textures/ScissorsUI/time_bubble.png",
        "time_bubble"
    );

    timerFrameImage->SetSize({ 298, 192 });
    timerFrameImage->SetPivot({ 0.5f, 0.5f });
    timerFrameImage->zOrder = 0;

    uiManager->Add(timerFrameImage);



}

void GameTimerUiActor::Update(float elapsedTime)
{
    // UI位置
    DirectX::XMFLOAT3 position = GetPosition();

    XMFLOAT2 uiPos = WorldToUI(position);

    auto gameManager =
        GetOwnerScene()->GetActorManager()->GetActorOfType<ScissorsGameManager>();

    if (!gameManager)
    {
        Logger::Error(U8("TimerUiActorでgameManagerがnullptrです。"));
        return;
    }

    float timer = gameManager->GetRequiredTime();

    // 秒に変換
    int totalSeconds = static_cast<int>(timer);

    UpdateTimerDigits(totalSeconds);

    // MM:SS
    // [0][1] : [2][3]

    timerDigits[0]->SetWorldPosition({ uiPos.x - spacing * 2.0f+ secondSpacing.x, uiPos.y+ minuteSpacing.y });
    timerDigits[1]->SetWorldPosition({ uiPos.x - spacing * 1.0f+ secondSpacing.x, uiPos.y+ minuteSpacing.y });


    timerFrameImage->SetWorldPosition({ uiPos.x, uiPos.y });

    timerDigits[2]->SetWorldPosition({ uiPos.x + spacing * 1.0f+ minuteSpacing.x, uiPos.y+secondSpacing.y });
    timerDigits[3]->SetWorldPosition({ uiPos.x + spacing * 2.0f+ minuteSpacing.x, uiPos.y+secondSpacing.y });

    for (int i = 0; i < 4; i++)
    {
        timerDigits[i]->SetSize(numberSize);
        timerDigits[i]->SetColor(DirectX::XMFLOAT4{ 0.471f,0.455f,0.498f,1.0f });
    }
}

void GameTimerUiActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::DragFloat(U8("数字の間"), &spacing);
    ImGui::DragFloat2(U8("数字の幅"), &numberSize.x);
    ImGui::DragFloat2(U8("分の間"), &minuteSpacing.x);
    ImGui::DragFloat2(U8("秒の間"), &secondSpacing.x);
#endif
}


void GameTimerUiActor::UpdateTimerDigits(int totalSeconds)
{
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    int minuteTens = minutes / 10;
    int minuteOnes = minutes % 10;

    int secondTens = seconds / 10;
    int secondOnes = seconds % 10;

    int numbers[4] =
    {
        minuteTens,
        minuteOnes,
        secondTens,
        secondOnes
    };

    for (int i = 0; i < 4; i++)
    {
        int digit = numbers[i];

        timerDigits[i]->SetUV({
            150.0f * digit,
            0.0f,
            150.0f,
            200.0f
            });

        bool visible = true;

        // 分の十の位
        if (i == 0 && digit == 0)
        {
            visible = false;
        }

        // 秒の十の位
        if (i == 2 && digit == 0)
        {
            visible = false;
        }

        timerDigits[i]->SetVisible(visible);
    }
}