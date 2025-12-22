#pragma once

enum class DebugDrawType :uint8_t
{
    Sphere,
    Box,
    Line,
    Capsule,
};

struct DebugDrawCommand
{
    DebugDrawType type;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 endPosition;
    DirectX::XMFLOAT3 size;
    DirectX::XMFLOAT4 color;
    float lifetime = 0.0f;
};

class DebugDrawManager
{
public:
    static void DrawSphere(
        const DirectX::XMFLOAT3& pos,
        float radius,
        const DirectX::XMFLOAT4& color,
        float life = 0.0f);

    static void DrawBox(
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& size,
        const DirectX::XMFLOAT4& color,
        float life = 0.0f);

    static void DrawCapsule(
        const DirectX::XMFLOAT3& startPos,
        const DirectX::XMFLOAT3& endPos,
        float radius,
        const DirectX::XMFLOAT4& color,
        float life = 0.0f);

    static void DrawLine(
        const DirectX::XMFLOAT3& startPos,
        const DirectX::XMFLOAT3& endPos,
        const DirectX::XMFLOAT4& color,
        float life = 0.0f);

    static void Tick(float deltaTime);
    static void Render(ID3D11DeviceContext* context);

private:
    static inline std::vector<DebugDrawCommand> commands_;
};
