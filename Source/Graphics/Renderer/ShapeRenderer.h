#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Graphics/Resource/GltfModel.h"

class ShapeRenderer
{
private:
    ShapeRenderer() = default;
    virtual ~ShapeRenderer() = default;

public:
    //static ShapeRenderer& Instance()
    //{
    //    static ShapeRenderer instance;
    //    return instance;
    //}

    static void Initialize(ID3D11Device* device);

    enum class Type :uint8_t
    {
        Line,
        Segment,
        Point
    };

    // ���`��
    static void DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& angle, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color);


    static void DrawBox(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3& size, const DirectX::XMFLOAT4& color);

    // ���`��
    static void DrawSphere(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, const DirectX::XMFLOAT4& color);

    // �J�v�Z���`��
    static void DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color);

    static void DrawCapsule(ID3D11DeviceContext* immediateContext,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT4& rotation, // �� �N�H�[�^�j�I���ǉ�
        float radius, float height,
        const DirectX::XMFLOAT4& color);

    static void DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition, float radius, const DirectX::XMFLOAT4& color);

    static void DrawCapsule(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& worldTransform, float radius, float height, const DirectX::XMFLOAT4& color);

    //���`��
    static  void DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type);

    //���`�� ����Ȃ�
    static void DrawSegment(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT3& startPosition, const DirectX::XMFLOAT3& endPosition);
private:
    struct DebugConstants
    {
        DirectX::XMFLOAT4 color;
    };

    static inline Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    static inline Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    static inline Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    static inline Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    static inline Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer[2];

    static inline const size_t maxPoints = 500;

    static inline std::unique_ptr<GltfModel> sphere = nullptr;
    static inline std::unique_ptr<GltfModel> topHalfSphere = nullptr;
    static inline std::unique_ptr<GltfModel> bottomHalfSphere = nullptr;
    static inline std::unique_ptr<GltfModel> cylinder = nullptr;
    static inline std::unique_ptr<GltfModel> capsule = nullptr;
    static inline std::unique_ptr<GltfModel> cube = nullptr;
};


//�f�o�b�N�����A�����A�_��`�悷��
class LineSegment
{
public:
    enum class Type
    {
        Line,
        Segment,
        Point
    };

    LineSegment(ID3D11Device* device, size_t maxSegments);

    virtual ~LineSegment() = default;

    void Draw(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4& viewProjection, const DirectX::XMFLOAT4& color, const std::vector<DirectX::XMFLOAT3>& points, Type type);
private:
    struct Constants
    {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT4 color;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    const size_t max_points;

};