#pragma once

// C++ 標準ライブラリ
#include <memory>
#include <string>


// 他ライブラリ
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// プロジェクトの他のヘッダ
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Core/PipelineState.h"
#include "Components/Base/SceneComponent.h"
#include "Graphics/Resource/InterleavedGltfModel.h"
#include "Engine/Utility/Win32Utils.h"
#include "Game/Actors/WaterSphere/MorphModel.h"
#include "Graphics/Resource/Texture.h"

class Actor;

//--　描画
class MeshComponent :public SceneComponent
{
public:
    PipeLineStateDesc pipeLineState_;   // これ使ってないから後で消す
    std::optional<std::string> overrideDeferredPipelineName;
    std::optional<std::string> overrideForwardPipelineName;
    std::optional<std::string> overrideCascadeShadowPipelineName;

    // このメッシュのレンダーパス
    enum class MeshRenderPass :uint8_t
    {
        Deferred,
        Forward
    };
    MeshRenderPass renderPass = MeshRenderPass::Deferred;
public:
    MeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner)
    {
        plusAlphaCBuffer = std::make_unique<ConstantBuffer<PlusAlphaConstants>>(Graphics::GetDevice());
    };
    std::shared_ptr<InterleavedGltfModel> model;
    // モデルのノード情報
    std::vector<InterleavedGltfModel::Node> modelNodes = {};

    virtual void Tick(float deltaTime)override
    {
    }

    DirectX::XMFLOAT3 GetModelSize() const
    {
        AABB aabb = model->GetAABB();
        return{ aabb.max.x - aabb.min.x,aabb.max.y - aabb.min.y,aabb.max.z - aabb.min.z };
    }

    virtual void SetModel(const std::string& fileName, bool isSaveVerticesData = false) = 0;

    virtual void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;
    virtual void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;
    virtual void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;

    virtual void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const = 0;

    virtual void SetIsVisible(bool isVisible) { this->isVisible_ = isVisible; }

    virtual bool IsVisible() const { return isVisible_; }

    // 他のメッシュコンポーネントに必要な外部からの定数バッファ更新するためのフック関数
    virtual void UpdateConstantBuffer(ID3D11DeviceContext* immediateContext) const {}

    void UpdatePlusAlphaConstants(ID3D11DeviceContext* immediateContext) const
    {
        plusAlphaCBuffer->data.hueShift = hueShift;
        plusAlphaCBuffer->data.saturation = saturation;
        plusAlphaCBuffer->data.brightness = brightness;
        plusAlphaCBuffer->data.dissolve = dissolve;
        plusAlphaCBuffer->data.cpuColor = cpuColor;
        plusAlphaCBuffer->data.emissionPower = emissionPower;
        plusAlphaCBuffer->data.value = value;
        plusAlphaCBuffer->Activate(immediateContext, 5);
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  model").c_str()))
        {
            ImGui::Checkbox("isVisible", &isVisible_);
            ImGui::SliderFloat("hueShift", &hueShift, 0.0f, +360.0f);
            ImGui::SliderFloat("saturation", &saturation, 0.1f, +2.0f);
            ImGui::SliderFloat("brightness", &brightness, 0.1f, +2.0f);
            ImGui::SliderFloat("dissolve", &dissolve, 0.0f, 1.0f);
            ImGui::ColorEdit4("cpuColor", &cpuColor.x);
            ImGui::SliderFloat("emissionPower", &emissionPower, 0.0f, 20.0f);
            ImGui::SliderFloat4("morphWeight", &plusAlphaCBuffer->data.morphWeights.x, 0.0f, 1.0f);
            ImGui::SliderInt("value", &plusAlphaCBuffer->data.value, 0, 10);
            ImGui::TreePop();
        }
#endif
    }

    void SetPipeLineState(const PipeLineStateDesc& pipelinesState) { this->pipeLineState_ = pipelinesState; }

    PipeLineStateDesc GetPipeLineState()const { return pipeLineState_; }

    void SetIsCastShadow(bool isCastShadow) { this->isCastShadow_ = isCastShadow; }

    virtual bool IsCastShadow() const { return isCastShadow_; }

    virtual void OnRegister() override {}

    // 数値が大きいほうが後に描画される
    void SetPriority(int priority) { this->priority = priority; }
    int GetPriority() const { return priority; }

    // モデルごとに更新したいPlusAlpha 用定数バッファ
    struct PlusAlphaConstants
    {
        DirectX::XMFLOAT4 cpuColor; // 色をCPU側で指定する用　（ダメージ当たったときとか）

        float	hueShift;	// 色相調整
        float	saturation;	// 彩度調整
        float	brightness;	// 明度調整
        float   dissolve;   // ディゾルブ用

        DirectX::XMFLOAT4 morphWeights = { 0.0f,0.0f,0.0f,0.0f };  // モーフモデルに使用する weight 0.0f ~ 1.0f

        float emissionPower; // 自己発光の強さ
        int value = 0;
    };
    std::unique_ptr<ConstantBuffer<PlusAlphaConstants>> plusAlphaCBuffer;

    float hueShift = 0.0f;	// 色相調整
    float saturation = 1.0f;	// 彩度調整
    float brightness = 1.0f;	// 明度調整
    float   dissolve = 0.0f;   // ディゾルブ用
    DirectX::XMFLOAT4 cpuColor = { 1.0f,1.0f,1.0f,1.0f }; // 色をCPU側で指定する用　（ダメージ当たったときとか）
    float emissionPower = 1.0f; // 自己発光の強さ
    float morphWeight = 0.0f;   // モーフモデルに使用する weight  0.0f ~ 1.0f 
    int value = 0;

protected:
    //描画するかどうか
    bool isVisible_ = true;
    // 影をつけるかどうか
    bool isCastShadow_ = true;
    // 描画優先度
    int priority = 0;


};

class SkeletalMeshComponent :public MeshComponent
{
public:
    SkeletalMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
    }

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, ModelTypes::ModelMode::SkeletalMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }


    void AppendAnimations(const std::vector<std::string>& filenames) const
    {
        //model->AddAnimations(filenames);
        model->AppendAnimations(filenames);
    }

    void Tick(float deltaTime)override
    {

    }

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

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        model->CastShadow(immediateContext, world, modelNodes);
    }

    DirectX::XMFLOAT3 GetJointWorldPosition(const std::string& name)
    {
        if (auto parent = attachParent_.lock())
        {
            DirectX::XMFLOAT4X4 parentWorld = parent->GetComponentWorldTransform().ToWorldTransform();
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
        return model->GetJointWorldPosition(name, modelNodes, world);
    }

private:

};


class MorphMeshComponent :public MeshComponent
{
public:
    MorphMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner) {}

    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<MorphModel>(device, filename, ModelTypes::ModelMode::SkeletalMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }


    void AppendAnimations(const std::vector<std::string>& filenames) const
    {
        model->AppendAnimations(filenames);
    }

    void Tick(float deltaTime)override
    {

    }

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

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->Render(immediateContext, world, model->nodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //model->CastShadow(immediateContext, world, model->nodes);
        model->CastShadow(immediateContext, world, modelNodes);
    }

    DirectX::XMFLOAT3 GetJointWorldPosition(const std::string& name)
    {
        if (auto parent = attachParent_.lock())
        {
            DirectX::XMFLOAT4X4 parentWorld = parent->GetComponentWorldTransform().ToWorldTransform();
            //return model->GetJointWorldPosition(name, model->nodes, parentWorld);
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        else
        {
            DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
            //return model->GetJointWorldPosition(name, model->nodes, world);
            return model->GetJointWorldPosition(name, modelNodes, world);
        }

        return { 0.0f,0.0f,0.0f };
    }


};


class StaticMeshComponent :public MeshComponent
{
public:
    StaticMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
    }


    void SetModel(const std::string& filename, bool isSaveVerticesData = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = std::make_shared<InterleavedGltfModel>(device, filename, ModelTypes::ModelMode::StaticMesh, isSaveVerticesData);
        modelNodes = model->GetNodes();
    }

    //void Update(float deltaTime)override {}

    void RenderOpaque(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Opaque, pipeLineState_);
    }
    void RenderMask(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Mask, pipeLineState_);
    }
    void RenderBlend(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->Render(immediateContext, world, modelNodes, InterleavedGltfModel::RenderPass::Blend, pipeLineState_);
    }

    void CastShadow(ID3D11DeviceContext* immediateContext, const DirectX::XMFLOAT4X4 world) const override
    {
        //const DirectX::XMFLOAT4X4 world = CreateWorldMatrix();
        model->CastShadow(immediateContext, world, modelNodes);
    }
};





