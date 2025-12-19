#pragma once
#include "Components/Render/MeshComponent.h"


class ElasticMeshComponent :public MeshComponent
{
public:
    ElasticMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
        overridePipelineName= "elasticBuilding";
        overrideCascadeShadowPipelineName = "CascadeShadowMapElasticBuilding";
    }

    ~ElasticMeshComponent() override = default;

    void SetModel(const std::string& filename, bool isSaveVerticesData = false) override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, InterleavedGltfModel::Mode::SkeltalMesh, isSaveVerticesData);
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

    DirectX::XMFLOAT3 GetJointWorldPosition(const std::string& name)
    {
        if (auto parent = attachParent_.lock())
        {
            DirectX::XMFLOAT4X4 parentWorld = parent->GetComponentWorldTransform().ToWorldTransform();
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        else
        {
            DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
            return model->GetJointWorldPosition(name, modelNodes, world);
        }


        return { 0.0f,0.0f,0.0f };
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI

        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  model").c_str()))
        {
            ImGui::Checkbox("isVisible", &isVisible_);
            ImGui::TreePop();
        }
#endif
    }

    struct ElasticConstants
    {
        DirectX::XMFLOAT4 p1; // 始点
        DirectX::XMFLOAT4 p2; // 制御点
        DirectX::XMFLOAT4 p3; // 終点
        float buildProgress; // 0.0 ~ 1.0  t
        float modelHeight; // モデルの高さ
    };

private:
    std::unique_ptr<ConstantBuffer<ElasticConstants>> elasticBuildingCBuffer;
    ElasticConstants elasticConstants{};
    float modelHeight = 0.0f;
};
