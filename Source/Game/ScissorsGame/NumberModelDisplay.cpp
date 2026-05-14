#include "pch.h"
#include "NumberModelDisplay.h"

#include "Core/Actor.h"

void DigitSlot::SetDigit(int value, bool enable)
{
    for (int i = 0; i < 10; i++)
    {
        numbers[i]->SetIsVisible(enable && i == value);
    }
}

void DigitSlot::SetParent(const std::string& parentName, Actor* owner, const std::string& baseName, const DirectX::XMFLOAT3& offset,bool isBackCover)
{
    for (int i = 0; i < 10; i++)
    {
        std::string name = baseName + "_" + std::to_string(i);

        numbers[i] = owner->AddComponent<SkeletalMeshComponent>(name, parentName);

        numbers[i]->SetModel("./Data/TeamModels/Number/NumberModel_" + std::to_string(i) + ".gltf", false, false);
        numbers[i]->SetRelativeLocationDirect(offset);
        if (isBackCover)
        {
            numbers[i]->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        }
        else
        {
        numbers[i]->SetRelativeEulerRotationDirect({ 0.0f,0.0f,180.0f });
        }
        numbers[i]->SetIsCastShadow(false);
        numbers[i]->SetIsVisible(false);
    }
}

// コンポーネントの親の名前、コンポーネント名、
void NumberDisplay::Initialize(Actor* owner, const std::string& parentName, const std::string& baseName, const DirectX::XMFLOAT3& startPos, int maxDigits, float spacing,bool isBackCover)
{
    digits.resize(maxDigits);

    for (int i = 0; i < maxDigits; i++)
    {
        DirectX::XMFLOAT3 offset={0.0f,0.0f,0.0f};
        if (isBackCover)
        {
            offset =
            {
                startPos.x + spacing * i,
                startPos.y,
                startPos.z
            };
        }
        else
        {
            offset =
            {
                startPos.x - spacing * i,
                startPos.y,
                startPos.z
            };
        }
       

        digits[i].SetParent(
            parentName,
            owner,
            baseName + "_" + std::to_string(i),
            offset,isBackCover
        );
    }
}

void NumberDisplay::SetValue(int value,int minDigits)
{
    currentValue = value;

    int temp = value;
    int digitCount = 0;

    // 桁数を数える
    do
    {
        digitCount++;
        temp /= 10;
    } while (temp > 0);

    // 最低桁数保証
    digitCount = std::max<int>(digitCount, minDigits);

    for (int i = 0; i < digits.size(); i++)
    {
        int digit = value % 10;

        bool visible = isVisible && i < digitCount;

        digits[i].SetDigit(digit, visible);

        value /= 10;
    }
}

void NumberDisplay::SetVisible(bool visible)
{
    isVisible = visible;

    SetValue(currentValue);
}