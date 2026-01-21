#pragma once


#include <memory>
#include <vector>
#include <functional>

#include "Components/Easing/CoreEasingComponent.h"
#include "Core/CoreColor.h"
#include "Core/Vector.h"
#include "Engine/Input/InputSystem.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Renderer/SpriteRenderer.h"
#include "Graphics/Sprite/Sprite.h"
#include "UI/Font.h"
#include "UI/FontManager.h"

class UICoreComponent
{
public:
    UICoreComponent(const std::string& name)
    {
        this->name = name;
    }

    virtual ~UICoreComponent() = default;
    virtual void Update(float dt) {}
    virtual void Draw(ID3D11DeviceContext* immediateContext) {}
    virtual void DrawTexts(ID3D11DeviceContext* immediateContext) {}
    virtual void DrawSceneChangeSprite(ID3D11DeviceContext* immediateContext) {}
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

    void MarkPendingKill() { isPendingKill = true; }

    bool IsPendingKill() const { return isPendingKill; }

    XMFLOAT2 GetWorldPosition() { return this->worldPosition; }

    // テキスト描画コンポーネントに使用している　テキストが更新された時に呼ばれる
    bool IsDirty() const { return dirty; }
    void ClearDirty() { dirty = false; }

public:
    int zOrder = 0; // 値が大きいほど手前に描画される

    bool dirty = true;// テキスト描画コンポーネントに使用している

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



    std::string name = "UICoreComponent";
    UICoreComponent* parent = nullptr;
    std::vector<UICoreComponent*> children;

    // アクターの削除予約
    bool isPendingKill = false;

};


class UIImageComponent : public UICoreComponent
{
public:
    UIImageComponent(const std::string& filename, const std::string& name) :UICoreComponent(name)
    {
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), std::wstring(filename.begin(), filename.end()).c_str());
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }

    UIImageComponent(const std::string& name) :UICoreComponent(name)
    {
        // ダミーテクスチャを設定
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/square.png");
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }

    UIImageComponent() = default;

    CoreColor color = CoreColor::White;

    void SetColor(const CoreColor color) { this->color = color; }

    void SetTexture(const std::shared_ptr<Sprite>& sprite)
    {
        texture = sprite;
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }

    void Draw(ID3D11DeviceContext* immediateContext) override
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

    // UV設定
    void SetUV(const SpriteUV& inUV) { uv = inUV; }

protected:
    std::shared_ptr<Sprite>  texture;

};


enum class UIButtonState :uint8_t
{
    Normal,
    Hovered,
    Pressed
};

class UIButtonComponent : public UIImageComponent
{
public:
    UIButtonComponent(const std::string& filename, const std::string& name) :UIImageComponent(filename, name) {}

    UIButtonComponent(const std::string& name) :UIImageComponent(name) {}

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

    UIGaugeComponent(const std::string& name) :UIImageComponent(name)
    {
        frameTexture = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/square.png");
    }

    void Draw(ID3D11DeviceContext* immediateContext) override
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

class UISceneChangeComponent : public UICoreComponent
{
public:
    UISceneChangeComponent(const std::string& filename, const std::string& name) :UICoreComponent(name)
    {
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), std::wstring(filename.begin(), filename.end()).c_str());
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }

    UISceneChangeComponent(const std::string& name) :UICoreComponent(name)
    {
        // ダミーテクスチャを設定
        texture = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/square.png");
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }

    UISceneChangeComponent() = default;

    CoreColor color = CoreColor::White;

    void SetColor(const CoreColor color) { this->color = color; }

    void SetTexture(const std::shared_ptr<Sprite>& sprite)
    {
        texture = sprite;
        uv.w = texture->GetTextureSize().x;
        uv.h = texture->GetTextureSize().y;
    }



    void DrawSceneChangeSprite(ID3D11DeviceContext* immediateContext) override
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

private:
    std::shared_ptr<Sprite>  texture;

};

class UITextComponent : public UICoreComponent
{
public:
    explicit UITextComponent(const std::string& name) : UICoreComponent(name)
    {
        zOrder = 10;
    }

    void SetText(const std::wstring& t)
    {
        //if (text == t)
        //    return;
        this->text = t;
        //dirty = true;
    }

    void DrawTexts(ID3D11DeviceContext* immediateContext) override
    {
        if (!visible) return;

        FontManager::GetUIFont()->Draw(worldPosition.x, worldPosition.y, text.c_str(), { 1,1,0,1 }, scale.x, pivot.x, pivot.y);
    }


    void SetColor(const CoreColor color) { this->color = color; }

protected:
    std::wstring text;
    CoreColor color = CoreColor::White;
};

class UITextPopup :public UITextComponent
{
public:
    explicit UITextPopup(const std::string& name) : UITextComponent(name)
    {
        easingRunner = std::make_unique<EasingRunner>();
    }

    void Update(float dt) override;

    void Play(const std::wstring& text/*, const DirectX::XMFLOAT2& startPos*/)
    {
        SetText(text);
        //worldPosition = startPos;

        TestEasingHandler handler;
        handler.AddEasing(TestEaseType::OutExp, 0.0f, 1.0f, 0.8f);


        handler.SetCompletedFunction([this]()
            {
                MarkPendingKill();
            });

        PropertyAccessor<float> accessor;
        accessor.getter = [this]() { return 1.0f; };
        accessor.setter = [this](float v)
            {
                float startPos = worldPosition.y;
                // 位置を動かす
                float endPos = worldPosition.y + 10.0f;
                worldPosition.y = std::lerp(startPos, endPos, v);

                // フェードアウト
                color.a = 1.0f - v;

                // スケール
                scale.x = std::lerp(0.8f, 1.2f, v);
            };

        easingRunner->StartHandler(handler, accessor);

    }

private:
    std::shared_ptr<EasingRunner> easingRunner;
};