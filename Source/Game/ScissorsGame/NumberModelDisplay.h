#pragma once
#include "Components/Render/MeshComponent.h"

// 一桁数字のクラス　
struct DigitSlot
{
    std::array<std::shared_ptr<SkeletalMeshComponent>, 10> numbers;

    void SetDigit(int value, bool enable);

    // 　つける親の名前　　
    void SetParent(const std::string& parentName, Actor* owner, const std::string& baseName, const DirectX::XMFLOAT3& offset, bool isBackCover);
};

// 数値のクラス
struct NumberDisplay
{
    std::vector<DigitSlot> digits;

    // コンポーネントの親の名前、コンポーネント名、
    void Initialize(Actor* owner, const std::string& parentName, const std::string& baseName, const DirectX::XMFLOAT3& startPos, int maxDigits, float spacing, bool isBackCover);

    // 数値を表示する 最低何桁表示するか
    void SetValue(int value, int minDigits = 1);


    void SetVisible(bool visible);

private:
    bool isVisible = true;
    int currentValue = 0;
};