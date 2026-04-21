#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"

class Trail
{
public:
    // 軌跡構造　CPUで更新するもの
    struct TrailPoint
    {
        DirectX::XMFLOAT3 position;
        float life; // 残り時間
    };
    std::vector<TrailPoint> trailPoints;

    void Initialize()
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(TrailVertex) * maxPoints);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        hr = Graphics::GetDevice()->CreateBuffer(&bufferDesc, NULL, vertexBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
             {"TEXCOORD",0,DXGI_FORMAT_R32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}, // alpha
            {"TEXCOORD",1,DXGI_FORMAT_R32G32_FLOAT,0,16,D3D11_INPUT_PER_VERTEX_DATA,0}, // uv
        };

        hr = CreateVsFromCSO(Graphics::GetDevice(), "./Shader/TrailVS.cso", vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = CreatePsFromCSO(Graphics::GetDevice(), "./Shader/TrailPS.cso", pixelShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    }


    void UpdateTrail(float deltaTime)
    {
        for (auto& p : trailPoints)
            p.life -= deltaTime;

        trailPoints.erase(std::remove_if(trailPoints.begin(), trailPoints.end(),
            [](const TrailPoint& p) {return p.life <= 0.0f; }), trailPoints.end());

        vertices.clear();

        for (size_t i = 1; i < trailPoints.size(); i++)
        {
#if 0
            float alpha = trailPoints[i].life / 0.5f;
            vertices.push_back({ trailPoints[i].position, alpha });
#else
            auto& previent = trailPoints[i - 1];
            auto& current = trailPoints[i];

            // 進行方向
            DirectX::XMVECTOR p0 = XMLoadFloat3(&previent.position);
            DirectX::XMVECTOR p1 = XMLoadFloat3(&current.position);
            DirectX::XMVECTOR dir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(p1, p0));

            // 横方向（XZ平面ならこれでOK）
            DirectX::XMVECTOR side = DirectX::XMVector3Cross(dir, DirectX::XMVectorSet(0, 1, 0, 0));
            side = DirectX::XMVector3Normalize(side);

            float width = 0.3f; // 太さ
            float alpha = current.life / 0.5f;

            DirectX::XMFLOAT3 left, right;

            DirectX::XMVECTOR leftVec = DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(side, width));
            DirectX::XMVECTOR rightVec = DirectX::XMVectorSubtract(p1, DirectX::XMVectorScale(side, width));

            XMStoreFloat3(&left, leftVec);
            XMStoreFloat3(&right, rightVec);

            vertices.push_back({ left, alpha });
            vertices.push_back({ right, alpha });

#endif // 0
        }
    }

    void Render(ID3D11DeviceContext* immediateContext)
    {
        HRESULT hr{ S_OK };
        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        std::memcpy(mappedSubresource.pData, vertices.data(), sizeof(TrailVertex) * vertices.size());
        immediateContext->Unmap(vertexBuffer.Get(), 0);

        UINT stride{ sizeof(TrailVertex) };
        UINT offset{ 0 };
        immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        immediateContext->VSSetShader(vertexShader.Get(), NULL, 0);
        immediateContext->PSSetShader(pixelShader.Get(), NULL, 0);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->Draw(static_cast<UINT>(vertices.size()), 0);
    }
private:
    // 頂点構造体　GPUに送るもの
    struct TrailVertex
    {
        DirectX::XMFLOAT3 position;
        float alpha;
        DirectX::XMFLOAT2 uv;
    };
    std::vector<TrailVertex> vertices;
    size_t maxPoints = 1500; /**< 内部で扱える最大頂点数 */


    // バッファ/シェーダ/入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};



#if 0 // これ線のみが出る
class Trail
{
public:
    // 軌跡構造　CPUで更新するもの
    struct TrailPoint
    {
        DirectX::XMFLOAT3 position;
        float life; // 残り時間
    };
    std::vector<TrailPoint> trailPoints;

    void Initialize()
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.ByteWidth = static_cast<UINT>(sizeof(TrailVertex) * maxPoints);
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        hr = Graphics::GetDevice()->CreateBuffer(&bufferDesc, NULL, vertexBuffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
        {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        };
        hr = CreateVsFromCSO(Graphics::GetDevice(), "./Shader/LineSegmentVS.cso", vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        hr = CreatePsFromCSO(Graphics::GetDevice(), "./Shader/LineSegmentPS.cso", pixelShader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    }


    void UpdateTrail(float deltaTime)
    {
        for (auto& p : trailPoints)
            p.life -= deltaTime;

        trailPoints.erase(std::remove_if(trailPoints.begin(), trailPoints.end(),
            [](const TrailPoint& p) {return p.life <= 0.0f; }), trailPoints.end());

        vertices.clear();

        for (size_t i = 1; i < trailPoints.size(); i++)
        {
            float alpha = trailPoints[i].life / 0.5f;
            vertices.push_back({ trailPoints[i].position, alpha });
        }
    }

    void Render(ID3D11DeviceContext* immediateContext)
    {
        HRESULT hr{ S_OK };
        D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
        hr = immediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        std::memcpy(mappedSubresource.pData, vertices.data(), sizeof(TrailVertex) * vertices.size());
        immediateContext->Unmap(vertexBuffer.Get(), 0);

        UINT stride{ sizeof(TrailVertex) };
        UINT offset{ 0 };
        immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

        immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        immediateContext->VSSetShader(vertexShader.Get(), NULL, 0);
        immediateContext->PSSetShader(pixelShader.Get(), NULL, 0);
        immediateContext->IASetInputLayout(inputLayout.Get());

        immediateContext->Draw(static_cast<UINT>(vertices.size()), 0);
    }
private:
    // 頂点構造体　GPUに送るもの
    struct TrailVertex
    {
        DirectX::XMFLOAT3 position;
        float alpha;
    };
    std::vector<TrailVertex> vertices;
    size_t maxPoints = 1500; /**< 内部で扱える最大頂点数 */


    // バッファ/シェーダ/入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};
#endif // 0 // これ線のみが出る
