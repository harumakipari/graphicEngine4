#pragma once
#include "Components/Render/MeshComponent.h"


class ElasticMeshComponent :public MeshComponent
{
public:
    ElasticMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
        overrideDeferredPipelineName = "elasticBuildingDeferred";
        overrideForwardPipelineName = "elasticBuildingForward";
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

    void AddImpulse(const DirectX::XMFLOAT3& impulse)
    {
        elasticParameters.momentumX += impulse.x / elasticParameters.mass;
        elasticParameters.momentumY += impulse.y / elasticParameters.mass;
        elasticParameters.momentumZ += impulse.z / elasticParameters.mass;
    }

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
        ImGui::SliderFloat("Stiffness", &elasticParameters.stiffness, 0.1f, 10.0f);
        ImGui::SliderFloat("Damping", &elasticParameters.damping, 0.1f, 0.99f);
        ImGui::SliderFloat("Mass", &elasticParameters.mass, 0.1f, 5.0f);
        ImGui::SliderFloat("momentumX", &elasticParameters.momentumX, -10.0f, 10.0f);
        ImGui::SliderFloat("momentumY", &elasticParameters.momentumY, -10.0f, 10.0f);
        ImGui::SliderFloat("momentumZ", &elasticParameters.momentumZ, -10.0f, 10.0f);
        ImGui::SliderFloat(" maxDist", &elasticParameters.maxDist, -100.0f, 120.0f);
        ImGui::SliderFloat(" maxAngleDegrees", &elasticParameters.maxAngleDegrees, 0.0f, 360.0f);
#endif
    }

    struct ElasticConstants
    {
        DirectX::XMFLOAT4 p1; // 始点
        DirectX::XMFLOAT4 p2; // 制御点
        DirectX::XMFLOAT4 p3; // 終点
        float maxAngleDegree; // 度以上曲がらない
        float modelHeight; // モデルの高さ
        float stretchRate;  // モデルの伸び率
    };

    struct ElasticParameters
    {
        float stiffness = 4.0f;     // 硬さ（戻る力）
        float damping = 0.95f;    // 減衰
        float mass = 1.0f;     // 重さ（外力用）
        float maxAngleDegrees = 100.0f;     // 最大変形量
        float momentumX = 0.0f;
        float momentumY = 0.0f;
        float momentumZ = 0.0f;
        float maxDist = 0.0f;
    };

private:
    void UpdatePushElastic(float deltaTime);

    void UpdatePullElastic(float deltaTime);


    std::unique_ptr<ConstantBuffer<ElasticConstants>> elasticBuildingCBuffer;
    ElasticConstants elasticConstants{};
    ElasticParameters elasticParameters;
    DirectX::XMFLOAT3 externalForce_{ 0,0,0 };
    float modelHeight = 0.0f;

    DirectX::XMFLOAT2 dragStartMousePos = { 0.0f,0.0f };
    bool isDragging = false;
    float baseStretchRate = 1.0f;
};
