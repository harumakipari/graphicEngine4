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

    void SetParent(const std::string& parentName, Actor* owner, const std::string& baseName, const DirectX::XMFLOAT3& offset)
    {
        for (int i = 0; i < 10; i++)
        {
            std::string name = baseName + "_" + std::to_string(i);

            numbers[i] = owner->AddComponent<SkeletalMeshComponent>(name, parentName);

            numbers[i]->SetModel("./Data/TeamModels/Number/NumberModel_" + std::to_string(i) + ".gltf", false, false);
            numbers[i]->SetRelativeLocationDirect(offset);
            numbers[i]->SetIsCastShadow(false);
            numbers[i]->SetIsVisible(false);
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

    // スコアを設定する
    void SetScore(int score);
private:

    DirectX::XMFLOAT3 scoreRelativePosition = { 0.0f,0.0f,0.f };

    std::vector<DigitSlot> scoreDigits;

    std::shared_ptr<SkeletalMeshComponent> numberModel;

};

