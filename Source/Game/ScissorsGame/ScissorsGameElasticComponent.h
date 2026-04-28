#pragma once
#include "Components/Render/MeshComponent.h"

class ScissorsGameElasticMeshComponent :public MeshComponent
{
public:
    enum class State
    {
        Idle,
        Tied,
        None,
    };

public:
    ScissorsGameElasticMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
        overrideDeferredPipelineName = "elasticBuildingDeferred";
        overrideForwardPipelineName = "elasticBuildingForward";
        overrideCascadeShadowPipelineName = "CascadeShadowMapElasticBuilding";
    }

    ~ScissorsGameElasticMeshComponent() override = default;

    void SetModel(const std::string& filename, bool isSaveVerticesData = false, bool convertToLHS = false) override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = AssetManager::Get().LoadModel(device, filename, ModelTypes::ModelMode::SkeletalMesh, false, convertToLHS);
        modelNodes = model->GetNodes();
        int a = 0;
    }


    void Initialize() override;

    void Tick(float deltaTime)override;

    void SetMaterialPS(const std::string& psFilename, const std::string& materialName) const
    {
        ID3D11Device* device = Graphics::GetDevice();
        for (InterleavedGltfModel::Material& material : model->materials)
        {
            if (material.name == materialName)
            {
                HRESULT hr = CreatePsFromCSO(device, psFilename.c_str(), material.replacedPixelShader.GetAddressOf());
                _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
            }
        }
    }

    void UpdateConstantBuffer(ID3D11DeviceContext* immediateContext) const override;

    // --- このあたりの関数を使っていないから後程削除する ---
    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world)const override
    {
        //model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->CastShadow(immediateContext, world, modelNodes);
    }

    virtual void DrawImGuiInspector() override;

    struct ElasticConstants
    {
        DirectX::XMFLOAT4 p1; // 始点
        DirectX::XMFLOAT4 p2; // 制御点
        DirectX::XMFLOAT4 p3; // 終点
        float maxAngleDegree; // 度以上曲がらない
        float modelHeight; // モデルの高さ
    };

    struct ElasticParameters
    {
        float stiffness = 10.0f;     // 硬さ（戻る力）
        float damping = 0.95f;    // 減衰
        float mass = 1.0f;     // 重さ（外力用）
        float maxAngleDegrees = 100.0f;     // 最大変形量
        float momentumX = 0.0f;
        float momentumY = 0.0f;
        float momentumZ = 0.0f;
        float maxDist = 3.45f;
    };

    void SetElasticEnabled(bool enabled);

    bool IsElasticEnabled() const { return elasticEnabled; }
private:
    std::unique_ptr<ConstantBuffer<ElasticConstants>> elasticBuildingCBuffer;
    ElasticConstants elasticConstants{};
    ElasticParameters elasticParameters;
    float modelHeight = 0.0f;

    DirectX::XMFLOAT3 p3Target;   // 入力・外力で決まる目標位置
    DirectX::XMFLOAT3 p3Current;  // 実際に描画やシェーダに渡す p3（結果）
    DirectX::XMFLOAT3 p3Base;

    float shakeAmp = 0.3f;            // 揺れの大きさ
    float shakeSpeed = 10.0f;          // 揺れる速さ
    float overshoot = 1.0f;           // オーバーシュート量
    bool elasticEnabled = true;

    State state = State::Idle;
    DirectX::XMFLOAT3 velocity = { 0.0f,0.0f,0.0f };

    DirectX::XMFLOAT3 shakePower = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 amplitude = { 0.0f,0.2f,0.0f }; // 揺れの大きさ
    float frequency = 5.6f; // 揺れの速さ

    float yLimitRatio = 0.7f;   // 下限
    float noisePhaseX = 1.0f;
    float noisePhaseY = 2.4f;
    float noisePhaseZ = 0.7f;
    float noiseFreqMulX = 1.0f;
    float noiseFreqMulZ = 1.3f;
};
