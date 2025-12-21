#include "pch.h"
#include "SpriteRenderer.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Sprite/Sprite.h"


void SpriteRenderer::Draw(
    Sprite* sprite,
    const DirectX::XMFLOAT2& position,
    const DirectX::XMFLOAT2& size,
    const DirectX::XMFLOAT4& color,
    float angle
)
{
    if (!sprite) return;

    sprite->Render(
        Graphics::GetDeviceContext(),
        position.x,
        position.y,
        size.x,
        size.y,
        color.x,
        color.y,
        color.z,
        color.w,
        angle
    );
}

void SpriteRenderer::Draw(
    Sprite* sprite,
    const DirectX::XMFLOAT2& position,
    const DirectX::XMFLOAT2& size,
    const DirectX::XMFLOAT4& color,
    const SpriteUV& uv,
    float angle,
    const DirectX::XMFLOAT2& pivot,
    const DirectX::XMFLOAT2& scale
)
{
    if (!sprite ) return;

    sprite->Render(
        Graphics::GetDeviceContext(),
        position.x,
        position.y,
        size.x,
        size.y,
        color,
        angle,
        uv.x,
        uv.y,
        uv.w,
        uv.h,
        scale.x,
        scale.y,
        pivot.x,
        pivot.y
    );
}