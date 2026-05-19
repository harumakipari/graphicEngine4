#pragma once
#include "Components/Render/MeshComponent.h"

// 一桁数字のクラス　
struct DigitSlot
{
    std::array<std::shared_ptr<SkeletalMeshComponent>, 10> numbers;

    void SetDigit(int value, bool enable);

    // 　つける親の名前　　
    void SetParent(const std::string& parentName, Actor* owner, const std::string& baseName, const DirectX::XMFLOAT3& offset, bool isBackCover);

    // 数字の色を変える
    void SetColor(const DirectX::XMFLOAT4& newColor);

    // 現在の色
    DirectX::XMFLOAT4 color = { 1,1,1,1 };
};

// 数値のクラス
struct NumberDisplay
{
    std::shared_ptr<SceneComponent> root;

    std::vector<DigitSlot> digits;

    // コンポーネントの親の名前、コンポーネント名、
    void Initialize(Actor* owner, const std::string& parentName, const std::string& baseName, const DirectX::XMFLOAT3& startPos, int maxDigits, float spacing, bool isBackCover);

    void Update(float deltaTime);

    // 数値を表示する 最低何桁表示するか
    void SetValue(int value, int minDigits = 1, bool popTrigger = true);

    void SetVisible(bool visible);

    void SetColor(const DirectX::XMFLOAT4& color);
private:
    bool isVisible = true;
    int currentValue = 0;
    DirectX::XMFLOAT3 baseScale = { 1.0f,1.0f,1.0f };
    float popScale = 1.0f;
    bool popTrigger = false;
    float popSpeed = 12.0f;
    float popStrength = 1.3f;

    int currentMinDigits = 0;
};