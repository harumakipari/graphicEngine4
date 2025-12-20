#include "pch.h"
#include "DebugDrawManager.h"

#include "Graphics/Renderer/ShapeRenderer.h"

void DebugDrawManager::DrawSphere(
    const DirectX::XMFLOAT3& pos,
    float radius,
    const DirectX::XMFLOAT4& color,
    float life)
{
    DebugDrawCommand command{};
    command.type = DebugDrawType::Sphere;
    command.position = pos;
    command.size = DirectX::XMFLOAT3{ radius, radius, radius };
    command.color = color;
    command.lifetime = life;
    commands_.push_back(command);
}

void DebugDrawManager::Tick(float deltaTime)
{
#if 0 // ライフタイムいる時に使用する
    for (auto it = commands_.begin(); it != commands_.end(); )
    {
        if (it->lifetime > 0.0f)
        {
            it->lifetime -= deltaTime;
            if (it->lifetime <= 0.0f)
            {
                it = commands_.erase(it);
                continue;
            }
        }
        ++it;
    }

#endif // 0 // ライフタイムいる時に使用する

    commands_.clear();

}

void DebugDrawManager::Render(ID3D11DeviceContext* immediateContext)
{
    for (auto& cmd : commands_)
    {
        switch (cmd.type)
        {
        case DebugDrawType::Sphere:
            ShapeRenderer::DrawSphere(
                immediateContext,
                cmd.position,
                cmd.size.x,
                cmd.color);
            break;
        case DebugDrawType::Box:
            ShapeRenderer::DrawBoxCenter(
                immediateContext,
                cmd.position,
                DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f },
                cmd.size,
                cmd.color);
            break;
        case DebugDrawType::Line:
            ShapeRenderer::DrawSegment(
                immediateContext,
                DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f },
                cmd.position);
            break;
        case DebugDrawType::Capsule:
            ShapeRenderer::DrawCapsule(
                immediateContext,
                cmd.position,
                cmd.size.x,
                cmd.size.y,
                cmd.color);
            break;
        default:
            break;
        }
    }
}