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

void DebugDrawManager::DrawBox(
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& size,
    const DirectX::XMFLOAT4& color,
    float life)
{
    DebugDrawCommand command{};
    command.type = DebugDrawType::Box;
    command.position = pos;
    command.size = size;
    command.color = color;
    command.lifetime = life;
    commands_.push_back(command);
}

void DebugDrawManager::DrawCapsule(
    const DirectX::XMFLOAT3& startPos,
    const DirectX::XMFLOAT3& endPos,
    float radius,
    const DirectX::XMFLOAT4& color,
    float life)
{
    DebugDrawCommand command{};
    command.type = DebugDrawType::Capsule;
    // カプセルの中心位置を計算
    command.position = DirectX::XMFLOAT3{
        (startPos.x + endPos.x) * 0.5f,
        (startPos.y + endPos.y) * 0.5f,
        (startPos.z + endPos.z) * 0.5f
    };
    // size.x に半径、size.y に高さを格納
    DirectX::XMVECTOR startVec = DirectX::XMLoadFloat3(&startPos);
    DirectX::XMVECTOR endVec = DirectX::XMLoadFloat3(&endPos);
    DirectX::XMVECTOR heightVec = DirectX::XMVectorSubtract(endVec, startVec);
    float height = DirectX::XMVectorGetX(DirectX::XMVector3Length(heightVec));
    command.size = DirectX::XMFLOAT3{ radius, height, 0.0f };
    command.color = color;
    command.lifetime = life;
    commands_.push_back(command);
}

void DebugDrawManager::DrawLine(
    const DirectX::XMFLOAT3& startPos,
    const DirectX::XMFLOAT3& endPos,
    const DirectX::XMFLOAT4& color,
    float life)
{
    DebugDrawCommand command{};
    command.type = DebugDrawType::Line;
    // 線の終点位置を position に格納
    command.position = startPos;
    command.endPosition = endPos;
    // size は未使用だが初期化しておく
    command.size = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
    command.color = color;
    command.lifetime = life;
    commands_.push_back(command);
}

void DebugDrawManager::DrawCylinder(
    const DirectX::XMFLOAT3& pos,
    float radius,
    float height,
    const DirectX::XMFLOAT4& color,
    float life)
{
    DebugDrawCommand command{};
    command.type = DebugDrawType::Capsule;
    command.position = pos;
    command.size = DirectX::XMFLOAT3{ radius, height, 0.0f };
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
            ShapeRenderer::DrawLineSegment(
                immediateContext,
                cmd.position,
                cmd.endPosition,cmd.color);
            break;
        case DebugDrawType::Capsule:
            ShapeRenderer::DrawCapsule(
                immediateContext,
                cmd.position,
                cmd.size.x,
                cmd.size.y,
                cmd.color);
            break;
        case DebugDrawType::Cylinder:
            ShapeRenderer::DrawCylinder(
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