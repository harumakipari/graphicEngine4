#pragma once
#include "StageData.h"
#include "Components/Easing/CoreEasingComponent.h"
#include "Core/Actor.h"
#include "BookBaseActor.h"

// 数字のクラス
struct DigitSlot
{
    std::array<std::shared_ptr<SkeletalMeshComponent>, 10> numbers;

    void SetDigit(int value)
    {
        for (int i = 0; i < 10; i++)
        {
            numbers[i]->SetIsVisible(i == value);
        }
    }
};

// リザルト本アクター
class ResultBookActor :public BookBaseActor
{
public:
    explicit ResultBookActor(const std::string& actorName) :BookBaseActor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    void DrawImGuiDetails() override;

private:

    DirectX::XMFLOAT3 scoreRelativePosition = { 0.0f,0.0f,0.f };

    std::vector<DigitSlot> scoreDigits;

    std::shared_ptr<SkeletalMeshComponent> numberModel;

};

