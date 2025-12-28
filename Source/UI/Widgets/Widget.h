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
    UICoreComponent(const std::string& filename, const std::string& name)
    {
        this->name = name;
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), std::wstring(filename.begin(), filename.end()).c_str());
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }
    UICoreComponent()
    {
        // ダミーテクスチャを設定
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/square.png");
    }

    virtual ~UICoreComponent() = default;
    virtual void Update(float dt) {}
    virtual void Draw() {}
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    virtual void OnMouseDown() {}
    virtual void OnMouseUp() {}
    virtual void OnClick() {}
    virtual void DrawImGui();

    void UpdateTransform();

    void SetParent(UICoreComponent* parent);
    const std::vector<UICoreComponent*>& GetChildren() const { return children; }

    void SetWorldPosition(const XMFLOAT2 worldPos) { this->worldPosition = worldPos; }

    void SetSize(const XMFLOAT2 size) { this->size = size; }

    void SetPivot(const XMFLOAT2 pivot) { this->pivot = pivot; }

    void SetWorldAngleDegree(float angle) { this->worldAngle = angle; }

    bool IsVisible() const { return visible; }

    bool IsEnabled() const { return enabled; }

    void SetVisible(const bool visible) { this->visible = visible; }

    void SetEnable(const bool enabled) { this->enabled = enabled; }

    void SetLocalPosition(const XMFLOAT2 localPos) { this->localPosition = localPos; }
protected:
    // スクリーン座標
    SpriteUV uv{ 0,0,100,100 };
    XMFLOAT2 scale = { 1.0f,1.0f };
    XMFLOAT2 worldPosition = { 0.0f,0.0f };
    XMFLOAT2 size = { 1.0f,1.0f };
    XMFLOAT2 pivot = { 0.0f,0.0f };
    bool visible = true;
    bool enabled = true;
    float worldAngle = 0.0f;

    // ローカル（親基準）
    XMFLOAT2 localPosition={ 0.0f,0.0f };
    float localAngle = 0.0f;

    std::shared_ptr<Sprite>  texture;


    std::string name = "UICoreComponent";
    UICoreComponent* parent = nullptr;
    std::vector<UICoreComponent*> children;
};


class UIImageComponent : public UICoreComponent
{
public:
    UIImageComponent(const std::string& filename, const std::string& name) :UICoreComponent(filename, name) {}

    UIImageComponent() = default;

    CoreColor color = CoreColor::White;

    void Draw() override
    {
        if (!visible) return;

        SpriteRenderer::Draw(
            texture.get(),
            worldPosition,
            size,
            color,
            uv,
            worldAngle,
            pivot,
            scale
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
    UIButtonComponent(const std::string& filename, const std::string& name) :UIImageComponent(filename, name) {}

    UIButtonComponent() = default;

    UIButtonState state = UIButtonState::Normal;

    std::function<void()> onClick;

    void Update(float dt) override
    {
        DirectX::XMFLOAT2 cursor = InputSystem::GetMousePositionScreen();
        if (!InputSystem::GetMousePositionUI(cursor))
        {
            state = UIButtonState::Normal;
            return;
        }
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
    bool IsInside(const DirectX::XMFLOAT2& p) const
    {
        return p.x >= worldPosition.x &&
            p.x <= worldPosition.x + size.x &&
            p.y >= worldPosition.y &&
            p.y <= worldPosition.y + size.y;
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
    UIGaugeComponent(const std::string& filename, const std::string& name) :UIImageComponent(filename, name) {}

    UIGaugeComponent() = default;

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
            worldPosition,
            drawSize,
            color,
            uv,
            worldAngle,
            pivot,
            scale
        );
    }
};
