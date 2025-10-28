#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <directxmath.h>

#include "Core/Actor.h"
#include "Graphics/Core/ConstantBuffer.h"
#include "Components/CollisionShape/ShapeComponent.h"
#include "Components/Render/MeshComponent.h"

#include "Engine/Utility/Win32Utils.h"
#include "Engine/Input/InputSystem.h"
struct ClothVertex
{
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 normal;
    DirectX::XMFLOAT4 tangent;
    DirectX::XMFLOAT4 texcoord;

    DirectX::XMFLOAT4 oldVelocity;
    DirectX::XMFLOAT4 velocity;
    DirectX::XMFLOAT4 newPosition;
};
struct ClothEdge
{
    uint32_t id = 0xffffff;
    float restLength = 0.0f;
    DirectX::XMFLOAT3 delta;
};


class SphereTest :public Actor
{
public:
    SphereTest(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Debug/Primitives/sphere.glb", false);

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        sphereCBuffer = std::make_unique<ConstantBuffer<SphereCBuffer>>(Graphics::GetDevice());

    }

    void Update(float deltaTime)override
    {
        using namespace DirectX;

        XMFLOAT3 pos = GetPosition();

        if (flag)
        {
            pos.y += deltaTime * -9.8f*0.1f;
        }
        else
        {
            pos.y = 10.0f;
        }
        //if (pos.y < 3.5f)
        //{
        //    pos.y = 3.5f;
        //}

        SetPosition(pos);

        if (onFloor)
        {
            pos.y = 1.0f;
            SetPosition(pos);
        }

        // flag がtrueのときだけ動く


        sphereCBuffer->data.radius = 1.1f;
        sphereCBuffer->data.worldPos = GetPosition();
        DirectX::XMFLOAT4X4 world = GetWorldTransform();
        XMMATRIX World = XMLoadFloat4x4(&world);
        XMMATRIX invWorldM = XMMatrixInverse(nullptr, World);
        DirectX::XMStoreFloat4x4(&sphereCBuffer->data.invWorldTransform, invWorldM);
        DirectX::XMStoreFloat4x4(&sphereCBuffer->data.worldTransform, World);
        sphereCBuffer->Activate(Graphics::GetDeviceContext(), 7);

        //clothMesh->Simulate(Graphics::GetDeviceContext());
    }

    struct SphereCBuffer
    {
        DirectX::XMFLOAT3 worldPos;
        float radius = 2.0f;
        DirectX::XMFLOAT4X4 worldTransform;
        DirectX::XMFLOAT4X4 invWorldTransform;
    };

    void DrawImGuiDetails()
    {
#ifdef USE_IMGUI
        ImGui::Checkbox("flag", &flag);
        ImGui::Checkbox("onFloor", &onFloor);
#endif
    };
    bool flag = false;
    bool onFloor = true;
private:
    std::unique_ptr<ConstantBuffer<SphereCBuffer>> sphereCBuffer;

};


class PlaneTest :public Actor
{
public:
    PlaneTest(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent1;
    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Models/TestCloth/cloth1.gltf", false);

        skeltalMeshComponent1 = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        skeltalMeshComponent1->SetModel("./Data/Models/TestCloth/cloth1.gltf", false);

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
        planeCBuffer = std::make_unique<ConstantBuffer<PlaneCBuffer>>(Graphics::GetDevice());

    }

    void Update(float deltaTime)override
    {
        using namespace DirectX;

        Plane plane1;
        plane1.normal = { 0.0f,1.0f,0.0f };
        plane1.d = 1.0f;

        Plane plane2;
        plane2.normal = { 0.0f,1.0f,0.0f };
        plane2.d = 0.0f;
        planeCBuffer->data.plane[1] = plane2;

        float angle = XMConvertToRadians(-30.0f);
        // Z軸回転行列
        XMMATRIX rotMatrix = XMMatrixRotationZ(angle);
        XMVECTOR n = XMLoadFloat3(&plane1.normal);
        n = XMVector3TransformNormal(n, rotMatrix);
        XMStoreFloat3(&plane1.normal, n);
        planeCBuffer->data.plane[0] = plane1;


        XMFLOAT3 pos1 = { plane1.normal.x * plane1.d, plane1.normal.y * plane1.d, plane1.normal.z * plane1.d };


        XMVECTOR base = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR normal = XMLoadFloat3(&plane1.normal);
        normal = XMVector3Normalize(normal);

        XMVECTOR axis = XMVector3Cross(base, normal);
        float dot = XMVectorGetX(XMVector3Dot(base, normal));
        float normalLength = XMVectorGetX(XMVector3Length(axis));

        XMVECTOR quat;
        if (normalLength < 1e-6f)
        {
            quat = (dot < 0.0f) ? XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), XM_PI) : XMQuaternionIdentity();
        }
        else
        {
            float angle2 = acosf(dot);
            quat = XMQuaternionRotationAxis(axis, angle2);
        }
        XMFLOAT4 quatF;
        XMStoreFloat4(&quatF, quat);
        skeltalMeshComponent->SetWorldLocationDirect(pos1);
        skeltalMeshComponent->SetWorldRotationDirect(quatF);
        planeCBuffer->Activate(Graphics::GetDeviceContext(), 6);
    }

    struct Plane
    {
        DirectX::XMFLOAT3 normal;
        float d;
    };

    struct PlaneCBuffer
    {
        Plane plane[4];
    };

    void DrawImGuiDetails()
    {
#ifdef USE_IMGUI
        ImGui::Checkbox("flag", &flag);
        ImGui::Checkbox("onFloor", &onFloor);
#endif
    };
    bool flag = false;
    bool onFloor = true;
private:
    std::unique_ptr<ConstantBuffer<PlaneCBuffer>> planeCBuffer;
};


class ClothMeshComponent :public MeshComponent
{
public:
    ClothMeshComponent(const std::string& name, std::shared_ptr<Actor> owner) :MeshComponent(name, owner)
    {
        isCloth = true;
    }

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::SkeltalMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
        InitFromModel(this);
        HRESULT hr =CreateCsFromCSO(device, "./Shader/ClothCS.cso", clothUpdateCS.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        CreateAndUpdateClothMesh(device);
        hr=CreateCsFromCSO(device, "./Shader/ClothSimulateCopyCS.cso", clothConstraintCS.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        CreateAndUpdateClothMesh(device);
    }

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const {}
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const {}
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const {}

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const {}

    std::vector<ClothVertex> vertices;
    std::vector<ClothEdge> edges;

    uint32_t CLOTH_EDGES_PER_VERTEX = 4; // 頂点あたりのエッジ数


private:
    void InitFromModel(MeshComponent* mesh)
    {
        const auto& model = mesh->model;

        // モデルの頂点データをClothVertexに変換して格納
        for (const auto& mesh : model->meshes)
        {
            for (const auto& primitive : mesh.primitives)
            {
                for (const auto& vertex : primitive.cachedVertices)
                {
                    ClothVertex clothVertex;
                    clothVertex.position = { vertex.position.x,vertex.position.y,vertex.position.z,1.0f };
                    clothVertex.normal = { vertex.normal.x,vertex.normal.y,vertex.normal.z,0.0f };
                    clothVertex.tangent = { vertex.tangent.x,vertex.tangent.y,vertex.tangent.z,0.0f };
                    clothVertex.texcoord = { vertex.texcoord.x, vertex.texcoord.y,0.0f,0.0f };
                    clothVertex.oldVelocity = { 0.0f, 0.0f, 0.0f,1.0f };
                    clothVertex.velocity = { 0.0f, 0.0f, 0.0f,1.0f };
                    clothVertex.newPosition = { vertex.position.x,vertex.position.y,vertex.position.z,0.0f }; // 初期位置を設定
                    vertices.push_back(clothVertex);
                }
            }
        }

        vertices[0].oldVelocity.x = 1.0;
        vertices[5].oldVelocity.x = 1.0;
        vertices[10].oldVelocity.x = 1.0;
        //vertices[0].isPinned = 1;
        //vertices[5].isPinned = 1;

        // エッジデータの生成
        edges.resize(vertices.size() * CLOTH_EDGES_PER_VERTEX);
        std::vector<std::vector<uint32_t>> edgeMap(vertices.size());

        // 三角形のインデックスからエッジを抽出
        for (const auto& mesh : model->meshes)
        {
            for (const auto& primitive : mesh.primitives)
            {
                const auto& indices = primitive.cachedIndices;
                for (size_t i = 0; i < indices.size(); i += 3)
                {
                    // ここちょっと確認
                    uint32_t idx0 = indices[i];
                    uint32_t idx1 = indices[i + 1];
                    uint32_t idx2 = indices[i + 2];
                    edgeMap[idx0].push_back(idx1);
                    edgeMap[idx0].push_back(idx2);
                    edgeMap[idx1].push_back(idx0);
                    edgeMap[idx1].push_back(idx2);
                    edgeMap[idx2].push_back(idx0);
                    edgeMap[idx2].push_back(idx1);
                }
            }
        }

        // restLengthを計算
        for (size_t i = 0; i < vertices.size(); i++)
        {
            const auto& vertex = vertices[i];
            size_t baseIndex = i * CLOTH_EDGES_PER_VERTEX;

            for (uint32_t j = 0; j < CLOTH_EDGES_PER_VERTEX; ++j)
            {
                if (j < edgeMap[i].size())
                {
                    uint32_t neighborIdx = edgeMap[i][j];
#if 1
                    //base.id = neighborIdx;
                    DirectX::XMVECTOR p0 = DirectX::XMLoadFloat4(&vertex.position);
                    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat4(&vertices[neighborIdx].position);
                    DirectX::XMVECTOR Dis = DirectX::XMVectorSubtract(p1, p0);
                    float restLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(Dis));
                    DirectX::XMFLOAT3 delta;
                    DirectX::XMStoreFloat3(&delta, Dis);
#endif // 0
                    edges[baseIndex + j].id = neighborIdx;
                    edges[baseIndex + j].restLength = restLength/* compute rest length */;
                    edges[baseIndex + j].delta = delta;
                }
                else
                {
                    edges[baseIndex + j].id = 0xffffffffu; // unused slot
                }
            }
        }
    }


    void CreateAndUpdateClothMesh(ID3D11Device* device)
    {
#if 0
        HRESULT hr;

        // 頂点用　
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(ClothVertex) * static_cast<UINT>(vertices.size());
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS /*| D3D11_BIND_VERTEX_BUFFER*/;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(ClothVertex);

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices.data();

        hr = device->CreateBuffer(&desc, &initData, preClothVertexBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //hr = device->CreateBuffer(&desc, &initData, currentClothVertexBuffer.GetAddressOf());
        //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // 読み込み用SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = static_cast<UINT>(vertices.size());
        // 前のフレームのSRV
        hr = device->CreateShaderResourceView(preClothVertexBuffer.Get(), &srvDesc, preVertexSRV.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        // 今のフレームのSRV
        //hr = device->CreateShaderResourceView(currentClothVertexBuffer.Get(), &srvDesc, currentVertexSRV.GetAddressOf());
        //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // 読み書き込み用のUAV
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = static_cast<UINT>(vertices.size());

        hr = device->CreateUnorderedAccessView(preClothVertexBuffer.Get(), &uavDesc, preVertexUAV.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        //hr = device->CreateUnorderedAccessView(currentClothVertexBuffer.Get(), &uavDesc, currentVertexUAV.GetAddressOf());
        //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#else
        HRESULT hr;
        // 頂点用　
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(ClothVertex) * static_cast<UINT>(vertices.size());
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS /*| D3D11_BIND_VERTEX_BUFFER*/;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(ClothVertex);

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices.data();

        hr = device->CreateBuffer(&desc, &initData, clothVertexBuffer[a].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = device->CreateBuffer(&desc, &initData, clothVertexBuffer[b].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // 読み込み用SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = static_cast<UINT>(vertices.size());
        // 前のフレームのSRV
        hr = device->CreateShaderResourceView(clothVertexBuffer[a].Get(), &srvDesc, clothSRV[a].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
        // 今のフレームのSRV
        hr = device->CreateShaderResourceView(clothVertexBuffer[b].Get(), &srvDesc, clothSRV[b].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        // 読み書き込み用のUAV
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = static_cast<UINT>(vertices.size());

        hr = device->CreateUnorderedAccessView(clothVertexBuffer[a].Get(), &uavDesc, clothUAV[a].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        hr = device->CreateUnorderedAccessView(clothVertexBuffer[b].Get(), &uavDesc, clothUAV[b].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#endif // 0

        // エッジ用
        desc.ByteWidth = sizeof(ClothEdge) * static_cast<UINT>(edges.size());
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(ClothEdge);
        initData.pSysMem = edges.data();

        hr = device->CreateBuffer(&desc, &initData, edgeBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

        srvDesc.Buffer.NumElements = static_cast<UINT>(edges.size());
        hr = device->CreateShaderResourceView(edgeBuffer.Get(), &srvDesc, edgeSRV.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    // 指定されたアライメントに合わせて数値を調整する関数
    UINT Align(UINT num, UINT alignment)
    {
        return (num + (alignment - 1)) & ~(alignment - 1);
    }

public:
    void Simulate(ID3D11DeviceContext* immediateContext)
    {
#if 0
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        immediateContext->VSSetShaderResources(0, 1, nullSRV);
        immediateContext->PSSetShaderResources(0, 1, nullSRV);

        // UAVをCSにバインド
        immediateContext->CSSetUnorderedAccessViews(0, 1, preVertexUAV.GetAddressOf(), NULL);
        // UAVにSRVをバインド
        immediateContext->CSSetShaderResources(0, 1, edgeSRV.GetAddressOf());
        // CSを設定
        immediateContext->CSSetShader(clothCS.Get(), NULL, 0);
        // ここ確認
        const UINT threadGroupCountX = Align(static_cast<UINT>(vertices.size()), NUMTHREAD_X) / NUMTHREAD_X;
        immediateContext->Dispatch(threadGroupCountX, 1, 1);
        // UAVを解除
        ID3D11UnorderedAccessView* nullUAV = {};
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
#if 0

        immediateContext->CSSetShaderResources(1, 1, edgeSRV.GetAddressOf());
        immediateContext->CSSetUnorderedAccessViews(0, 1, vertexUAV.GetAddressOf(), NULL);
        immediateContext->CSSetShader(clothSimulateCS.Get(), NULL, 0);
        // ここ確認
        immediateContext->Dispatch(threadGroupCountX, 1, 1);

#endif // 0
        // UAVを解除
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
        immediateContext->CSSetShaderResources(0, 1, nullSRV);
#else
#if 0
        // VS で使ってた SRV を解除する
        {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            immediateContext->VSSetShaderResources(0, 1, nullSRV);
            immediateContext->PSSetShaderResources(0, 1, nullSRV);
        }

        ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
        ID3D11ShaderResourceView* nullSRVs[3] = { nullptr, nullptr ,nullptr };
        ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };

        UINT threadGroupCountX = Align((UINT)vertices.size(), NUMTHREAD_X) / NUMTHREAD_X;

        // --- CS①
        ID3D11ShaderResourceView* cs1SRVs[2] = { preVertexSRV.Get(), edgeSRV.Get() };
        immediateContext->CSSetShaderResources(0, 2, cs1SRVs);
        immediateContext->CSSetUnorderedAccessViews(0, 1, currentVertexUAV.GetAddressOf(), nullptr);
        immediateContext->CSSetShader(clothCS.Get(), nullptr, 0);

        immediateContext->Dispatch(threadGroupCountX, 1, 1);

        // バインドを外す
        immediateContext->CSSetShaderResources(0, 2, nullSRV);
        immediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

        // 入れ替え
        //std::swap(preClothVertexBuffer, currentClothVertexBuffer);
        //std::swap(preVertexSRV, currentVertexSRV);
        //std::swap(preVertexUAV, currentVertexUAV);

        // --- CS②
        //ID3D11ShaderResourceView* cs2SRVs[3] = { preVertexSRV.Get(),edgeSRV.Get() };
        //immediateContext->CSSetShaderResources(0, 3, cs2SRVs);
        //immediateContext->CSSetUnorderedAccessViews(0, 1, currentVertexUAV.GetAddressOf(), nullptr);
        //immediateContext->CSSetShader(clothSimulateCS.Get(), nullptr, 0);

        //immediateContext->Dispatch(threadGroupCountX, 1, 1);

        //// はず
        //immediateContext->CSSetShaderResources(0, 3, nullSRVs);
        //immediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

#endif // 0
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        immediateContext->VSSetShaderResources(0, 1, nullSRV);
        immediateContext->PSSetShaderResources(0, 1, nullSRV);

        //----Update

        // UAVをCSにバインド
        immediateContext->CSSetUnorderedAccessViews(0, 1, clothUAV[b].GetAddressOf(), NULL);
        // SRVをCSにバインド
        immediateContext->CSSetShaderResources(0, 1, clothSRV[a].GetAddressOf());
        immediateContext->CSSetShaderResources(1, 1, edgeSRV.GetAddressOf());
        // CSを設定
        immediateContext->CSSetShader(clothUpdateCS.Get(), NULL, 0);
        // ここ確認
        const UINT threadGroupCountX = Align(static_cast<UINT>(vertices.size()), NUMTHREAD_X) / NUMTHREAD_X;
        immediateContext->Dispatch(threadGroupCountX, 1, 1);
        // UAVを解除
        ID3D11UnorderedAccessView* nullUAV = {};
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
        // SRVを解除
        immediateContext->CSSetShaderResources(0, 1, nullSRV);
        immediateContext->CSSetShaderResources(1, 1, nullSRV);

        //----Constraint

        // UAVをCSにバインド
        immediateContext->CSSetUnorderedAccessViews(0, 1, clothUAV[b].GetAddressOf(), NULL);
        // SRVをCSにバインド
        immediateContext->CSSetShaderResources(0, 1, clothSRV[a].GetAddressOf());
        immediateContext->CSSetShaderResources(1, 1, edgeSRV.GetAddressOf());
        // CSを設定
        immediateContext->CSSetShader(clothConstraintCS.Get(), NULL, 0);
        // ここ確認
        immediateContext->Dispatch(threadGroupCountX, 1, 1);
        // UAVを解除
        immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
        // SRVを解除
        immediateContext->CSSetShaderResources(0, 1, nullSRV);
        immediateContext->CSSetShaderResources(1, 1, nullSRV);


        // ここでスワップする
        std::swap(a, b);

#if 0

        immediateContext->CSSetShaderResources(1, 1, edgeSRV.GetAddressOf());
        immediateContext->CSSetUnorderedAccessViews(0, 1, vertexUAV.GetAddressOf(), NULL);
        immediateContext->CSSetShader(clothSimulateCS.Get(), NULL, 0);
        // ここ確認
        immediateContext->Dispatch(threadGroupCountX, 1, 1);

#endif // 0
        //// UAVを解除
        //immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, NULL);
        //immediateContext->CSSetShaderResources(0, 1, nullSRV);


#endif // 0
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> clothSRV[2];
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> clothUAV[2];
    Microsoft::WRL::ComPtr<ID3D11Buffer> clothVertexBuffer[2];

    int a = 0;
    int b = 1;

    // ClothMesh GPU　用バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> preClothVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> preVertexSRV;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> preVertexUAV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> currentClothVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> currentVertexSRV;
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> currentVertexUAV;

    Microsoft::WRL::ComPtr<ID3D11Buffer> edgeBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> edgeSRV;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> clothUpdateCS;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> clothConstraintCS;
};

class Cloth : public Actor
{
public:
    Cloth(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
        //skeltalMeshComponent->SetModel("./Data/Models/TestCloth/cloth1.gltf", true);
        skeltalMeshComponent->SetModel("./Data/Models/TestCloth/cloth1.gltf", true);
        skeltalMeshComponent->overridePipelineName = "cloth";
        //skeltalMeshComponent->SetIsVisible(false);

        //clothMesh = this->NewSceneComponent<class ClothMeshComponent>("clothComponent", "skeltalComponent");
        //clothMesh->SetModel("./Data/Models/TestCloth/cloth1.gltf", true);
        //clothMesh->overridePipelineName = "cloth";


        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());


        //clothMesh = std::make_unique<ClothMeshComponent>(Graphics::GetDevice(), skeltalMeshComponent.get());

    }

    void Update(float deltaTime)override
    {
        using namespace DirectX;
        //clothMesh->Simulate(Graphics::GetDeviceContext());

        if (InputSystem::GetInputState("Enter"))
        {
            SetPosition({ 0.0f,10.0f,0.0f });
        }
    }


private:
    std::shared_ptr<ClothMeshComponent> clothMesh;

};