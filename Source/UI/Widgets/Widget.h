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

    void SetScale(const XMFLOAT2 scale) { this->scale = scale; }

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
    XMFLOAT2 localPosition = { 0.0f,0.0f };
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

    void SetColor(const CoreColor color) { this->color = color; }

    void Draw() override
    {
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

    void Update(float dt) override;

    void OnClick() override
    {
        if (onClick) onClick();
    }

private:
    bool IsInside(const DirectX::XMFLOAT2& p) const
    {
        // pivot を考慮した左上座標
        float left = worldPosition.x - size.x * pivot.x;
        float top = worldPosition.y - size.y * pivot.y;

        float right = left + size.x;
        float bottom = top + size.y;

        return p.x >= left && p.x <= right &&
            p.y >= top && p.y <= bottom;
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
    UIGaugeComponent(const std::string& frameFilename, const std::string& fillFilename, const std::string& name) :UIImageComponent(fillFilename, name)
    {
        frameTexture = std::make_unique<Sprite>(Graphics::GetDevice(), std::wstring(frameFilename.begin(), frameFilename.end()).c_str());
    }

    UIGaugeComponent() = default;

    void Draw() override
    {
        XMFLOAT2 drawSize = size;

        if (horizontal)
            drawSize.x *= value;
        else
            drawSize.y *= value;

        // 枠の描画
        SpriteRenderer::Draw(
            frameTexture.get(),
            worldPosition,
            size,
            color,
            uv,
            worldAngle,
            pivot,
            scale
        );


        // ゲージの中身
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


    void SetValue(const float current, const float max)
    {
        value = std::clamp(current / max, 0.0f, 1.0f);
    }

    bool horizontal = true;

private:
    float value = 1.0f;  // 0.0f ~ 1.0f
    std::shared_ptr<Sprite>  frameTexture;  //　枠のテクスチャ
};
