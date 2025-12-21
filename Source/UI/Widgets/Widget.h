#pragma once


#include <memory>
#include <vector>
#include <functional>

#include "Core/CoreColor.h"
#include "Core/Vector.h"
#include "Engine/Input/InputSystem.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Renderer/SpriteRenderer.h"
#include "Graphics/Sprite/Sprite.h"
#include "Widgets/Color.h"

class UICoreComponent
{
public:
    virtual ~UICoreComponent() = default;
    virtual void Update(float dt) {}
    virtual void Draw() {}
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    virtual void OnMouseDown() {}
    virtual void OnMouseUp() {}
    virtual void OnClick() {}

    void SetParent(UICoreComponent* parent);
    const std::vector<UICoreComponent*>& GetChildren() const;

    // スクリーン座標
    XMFLOAT2 position;
    XMFLOAT2 size;
    bool visible = true;
    bool enabled = true;

protected:
    UICoreComponent* parent = nullptr;
    std::vector<UICoreComponent*> children;
};


class UIImageComponent : public UICoreComponent
{
public:
    std::shared_ptr<Sprite>  texture;
    CoreColor color = CoreColor::White;

    void Draw() override
    {
        if (!visible) return;

        SpriteRenderer::Draw(
            texture.get(),
            position,
            size,
            color
        );
    }
};


enum class UIButtonState
{
    Normal,
    Hovered,
    Pressed
};

class UIButtonComponent : public UIImageComponent
{
public:
    UIButtonState state = UIButtonState::Normal;

    std::function<void()> onClick;

    void Update(float dt) override
    {
        DirectX::XMFLOAT2 cursor = InputSystem::GetMousePosition();

        bool inside = IsInside(cursor);

        if (inside)
        {
            // マウスカーソルを取得
            if (InputSystem::GetInputState("MouseLeft"))
            {// 左ボタンを押している間
                state = UIButtonState::Pressed;
            }
            else
            {
                if (state == UIButtonState::Pressed &&
                    InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
                {
                    OnClick();
                }
                state = UIButtonState::Hovered;
            }
        }
        else
        {
            state = UIButtonState::Normal;
        }

        UpdateVisual();
    }

    void OnClick() override
    {
        if (onClick) onClick();
    }

private:
    bool IsInside(const DirectX::XMFLOAT2& p)
    {
        return p.x >= position.x &&
            p.x <= position.x + size.x &&
            p.y >= position.y &&
            p.y <= position.y + size.y;
    }

    void UpdateVisual()
    {
        switch (state)
        {
        case UIButtonState::Normal:  color = CoreColor::White; break;
        case UIButtonState::Hovered: color = CoreColor(0.8f, 0.8f, 0.8f, 1); break;
        case UIButtonState::Pressed: color = CoreColor(0.8f, 0.8f, 0.8f, 1); break;
        }
    }
};


class UIGaugeComponent : public UIImageComponent
{
public:
    float value = 1.0f;  // 0.0f ~ 1.0f
    bool horizontal = true;

    void Draw() override
    {

        XMFLOAT2 drawSize = size;

        if (horizontal)
            drawSize.x *= value;
        else
            drawSize.y *= value;

        SpriteRenderer::Draw(
            texture.get(),
            position,
            drawSize,
            color
        );
    }
};
