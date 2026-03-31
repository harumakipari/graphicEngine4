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
#include "Core/Vector.h"
#include "Graphics/Resource/InterleavedGltfModel.h"
#include "Engine/Utility/Win32Utils.h"
#include "Game/Actors/WaterSphere/MorphModel.h"
#include "Graphics/Resource/AssetManager.h"
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

    virtual void SetModel(const std::string& fileName, bool isSaveVerticesData = false, bool convertToLHS = false) = 0;

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
        plusAlphaCBuffer->data.dissolve = dissolve;
        plusAlphaCBuffer->Activate(immediateContext, 5);
    }

    virtual void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        SceneComponent::DrawImGuiInspector();
        if (ImGui::TreeNode((name_ + "  model").c_str()))
        {
            ImGui::Checkbox("isVisible", &isVisible_);
            ImGui::SliderFloat("hueShift", &plusAlphaCBuffer->data.hueShift, 0.0f, +360.0f);
            ImGui::SliderFloat("saturation", &plusAlphaCBuffer->data.saturation, 0.1f, +5.0f);
            ImGui::SliderFloat("brightness", &plusAlphaCBuffer->data.brightness, 0.1f, +5.0f);
            ImGui::SliderFloat("dissolve", &dissolve, 0.0f, 1.0f);
            ImGui::ColorEdit4("cpuColor", &plusAlphaCBuffer->data.cpuColor.x);
            ImGui::SliderFloat("emissionPower", &plusAlphaCBuffer->data.emissionPower, 0.0f, 50.0f);
            ImGui::SliderFloat4("morphWeight", &plusAlphaCBuffer->data.morphWeights.x, 0.0f, 1.0f);
            const char* objectTypes[] =
            {
                "Default",
                "Player",
                "Enemy",
                "Stage",
            };

            int type = static_cast<int>(plusAlphaCBuffer->data.objectType);

            if (ImGui::Combo("Render Step", &type, objectTypes, IM_ARRAYSIZE(objectTypes)))
            {
                plusAlphaCBuffer->data.objectType = static_cast<ObjectType>(type);
            }

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

        float hueShift = 0.0f;	// 色相調整
        float saturation = 1.0f;	// 彩度調整
        float brightness = 1.0f;	// 明度調整
        float   dissolve;   // ディゾルブ用

        DirectX::XMFLOAT4 morphWeights = { 0.0f,0.0f,0.0f,0.0f };  // モーフモデルに使用する weight 0.0f ~ 1.0f

        float emissionPower; // 自己発光の強さ
        ObjectType objectType = ObjectType::Default; // オブジェクトの種類
    };
    std::unique_ptr<ConstantBuffer<PlusAlphaConstants>> plusAlphaCBuffer;

    float   dissolve = 0.0f;   // ディゾルブ用
    float morphWeight = 0.0f;   // モーフモデルに使用する weight  0.0f ~ 1.0f 

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
    struct ClothBone
    {
        int nodeIndex;

        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 prevPosition;

        float lengthToParent;
        DirectX::XMFLOAT3 restDir;
    };
    std::vector<ClothBone> clothBones;
    std::vector<std::vector<ClothBone>> clothChains;

    struct ClothConstraint
    {
        int chainA;
        int indexA;

        int chainB;
        int indexB;

        float restLength;
    };

    std::vector<ClothConstraint> horizontalConstraints;
    ClothBone CreateClothBone(int nodeIndex)
    {
        using namespace DirectX;

        ClothBone bone{};
        bone.nodeIndex = nodeIndex;

        const auto& node = modelNodes[nodeIndex];

        XMMATRIX M = XMLoadFloat4x4(&node.globalTransform);
        XMVECTOR pos = XMVector3TransformCoord(XMVectorZero(), M);

        XMStoreFloat3(&bone.position, pos);
        bone.prevPosition = bone.position;

        return bone;
    }

    void InitializeCloth()
    {
#if 1
        clothChains.clear();

        // --- 前（中央） ---
        BuildClothChain("loincloth_01");

        // --- 後ろ（中央） ---
        BuildClothChain("loincloth_back_01");

        // --- 後ろ下（別層）---
        BuildClothChain("loincloth_back_bottom_05");

        //// --- 左右 ---
        BuildClothChain("hip_cloth_root_l");
        BuildClothChain("hip_cloth_bottom_l");
        BuildClothChain("hip_cloth_root_r");

#else
        clothBones.clear();

        int root = model->FindNodeIndexByName("hip_cloth_root_l");
        int bottom = model->FindNodeIndexByName("hip_cloth_bottom_l");
        int end = model->FindNodeIndexByName("hip_cloth_bottomEnd_l");

        clothBones.push_back(CreateClothBone(root));
        clothBones.push_back(CreateClothBone(bottom));
        clothBones.push_back(CreateClothBone(end));

        using namespace DirectX;

        // 長さ計算
        for (int i = 1; i < clothBones.size(); ++i)
        {
            auto& parent = clothBones[i - 1];
            auto& child = clothBones[i];

            XMVECTOR p = XMLoadFloat3(&parent.position);
            XMVECTOR c = XMLoadFloat3(&child.position);

            XMVECTOR dir = XMVectorSubtract(c, p);

            // 長さ
            child.lengthToParent = XMVectorGetX(XMVector3Length(dir));

            // restDir作成（ローカル空間）
            dir = XMVector3Normalize(dir);
            XMStoreFloat3(&child.restDir, dir);
        }
#endif // 0
        BuildHorizontalConstraints();
    }

    void BuildHorizontalConstraints()
    {
        horizontalConstraints.clear();

        // チェーン同士を横につなぐ
        for (int i = 0; i < clothChains.size(); ++i)
        {
            for (int j = i + 1; j < clothChains.size(); ++j)
            {
                auto& chainA = clothChains[i];
                auto& chainB = clothChains[j];

                int minSize = std::min<int>(chainA.size(), chainB.size());

                for (int k = 1; k < minSize; ++k) // rootは除く
                {
                    using namespace DirectX;

                    XMVECTOR a = MathHelper::Load(chainA[k].position);
                    XMVECTOR b = MathHelper::Load(chainB[k].position);

                    float dist = XMVectorGetX(XMVector3Length(a - b));

                    ClothConstraint c;
                    c.chainA = i;
                    c.indexA = k;
                    c.chainB = j;
                    c.indexB = k;
                    c.restLength = dist;

                    horizontalConstraints.push_back(c);
                }
            }
        }
    }


    void BuildClothChain(const std::string& rootName)
    {

        using namespace DirectX;

        std::vector<ClothBone> chain;

        int current = model->FindNodeIndexByName(rootName);

        while (true)
        {
            chain.push_back(CreateClothBone(current));

            const auto& node = modelNodes[current];

            if (node.children.empty())
                break;

            current = node.children[0];
        }

        // 長さ & restDir
        for (int i = 1; i < chain.size(); ++i)
        {
            auto& parent = chain[i - 1];
            auto& child = chain[i];

            XMVECTOR p = XMLoadFloat3(&parent.position);
            XMVECTOR c = XMLoadFloat3(&child.position);

            XMVECTOR dir = XMVectorSubtract(c, p);

            child.lengthToParent = XMVectorGetX(XMVector3Length(dir));

            dir = XMVector3Normalize(dir);
            XMStoreFloat3(&child.restDir, dir);
        }
        Logger::Log("CHAIN: " + rootName + " size = " + std::to_string(chain.size()));

        clothChains.push_back(chain);
    }

    void UpdateGlobalTransforms()
    {
        using namespace DirectX;

        std::function<void(int, XMMATRIX)> traverse;

        traverse = [&](int nodeIndex, XMMATRIX parentMatrix)
            {
                auto& node = modelNodes[nodeIndex];

                XMMATRIX T = XMMatrixTranslation(
                    node.translation.x,
                    node.translation.y,
                    node.translation.z);

                XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation));

                XMMATRIX S = XMMatrixScaling(
                    node.scale.x,
                    node.scale.y,
                    node.scale.z);

                XMMATRIX local = S * R * T;

                XMMATRIX global = local * parentMatrix;

                XMStoreFloat4x4(&node.globalTransform, global);

                for (int child : node.children)
                {
                    traverse(child, global);
                }
            };

        for (int root : model->scenes[model->defaultScene].nodes)
        {
            traverse(root, XMMatrixIdentity());
        }
    }


    void UpdateCloth(float dt)
    {
        using namespace DirectX;

        XMVECTOR gravity = XMVectorSet(0, -9.8f, 0, 0);

        for (auto& chain : clothChains)
        {
            // --- root固定 ---
            {
                auto& root = chain[0];
                auto& node = modelNodes[root.nodeIndex];

                XMMATRIX M = XMLoadFloat4x4(&node.globalTransform);
                XMVECTOR pos = XMVector3TransformCoord(XMVectorZero(), M);

                root.position = MathHelper::StoreFloat3(pos);
                root.prevPosition = root.position;
            }

            // --- Verlet ---
            for (int i = 1; i < chain.size(); ++i)
            {
                auto& bone = chain[i];

                XMVECTOR pos = MathHelper::Load(bone.position);
                XMVECTOR prev = MathHelper::Load(bone.prevPosition);

                XMVECTOR velocity = XMVectorSubtract(pos, prev);
                velocity = XMVectorScale(velocity, 0.98f);

                bone.prevPosition = bone.position;

                pos = XMVectorAdd(pos, velocity);
                pos = XMVectorAdd(pos, XMVectorScale(gravity, dt * dt));

                bone.position = MathHelper::StoreFloat3(pos);
            }

            // --- 制約 ---
            for (int iter = 0; iter < 4; ++iter)
            {
                for (int i = 1; i < chain.size(); ++i)
                {
                    auto& parent = chain[i - 1];
                    auto& child = chain[i];

                    XMVECTOR p = MathHelper::Load(parent.position);
                    XMVECTOR c = MathHelper::Load(child.position);

                    XMVECTOR dir = XMVector3Normalize(c - p);

                    c = XMVectorAdd(p, XMVectorScale(dir, child.lengthToParent));

                    child.position = MathHelper::StoreFloat3(c);
                }
            }

            // --- 横制約 ---
            for (int iter = 0; iter < 4; ++iter)
            {
                for (auto& c : horizontalConstraints)
                {
                    auto& boneA = clothChains[c.chainA][c.indexA];
                    auto& boneB = clothChains[c.chainB][c.indexB];

                    using namespace DirectX;

                    XMVECTOR a = MathHelper::Load(boneA.position);
                    XMVECTOR b = MathHelper::Load(boneB.position);

                    XMVECTOR delta = b - a;
                    float len = XMVectorGetX(XMVector3Length(delta));

                    if (len < 0.0001f) continue;

                    float diff = (len - c.restLength) / len;

                    XMVECTOR offset = XMVectorScale(delta, 0.5f * diff);

                    // rootは動かさない
                    if (c.indexA != 0)
                        boneA.position = MathHelper::StoreFloat3(a + offset);

                    if (c.indexB != 0)
                        boneB.position = MathHelper::StoreFloat3(b - offset);
                }
            }

            ApplyClothToBones(chain);
        }
    }

    DirectX::XMVECTOR FromToRotation(DirectX::XMVECTOR from, DirectX::XMVECTOR to)
    {
        using namespace DirectX;

        from = XMVector3Normalize(from);
        to = XMVector3Normalize(to);

        float dot = XMVectorGetX(XMVector3Dot(from, to));

        // 同じ方向
        if (dot > 0.9999f)
        {
            return XMQuaternionIdentity();
        }

        // 逆方向（危険ゾーン）
        if (dot < -0.9999f)
        {
            XMVECTOR axis = XMVector3Cross(from, XMVectorSet(1, 0, 0, 0));

            if (XMVectorGetX(XMVector3LengthSq(axis)) < 0.0001f)
            {
                axis = XMVector3Cross(from, XMVectorSet(0, 1, 0, 0));
            }

            axis = XMVector3Normalize(axis);
            return XMQuaternionRotationAxis(axis, XM_PI);
        }

        XMVECTOR axis = XMVector3Cross(from, to);
        float angle = acosf(dot);

        return XMQuaternionRotationAxis(XMVector3Normalize(axis), angle);
    }

    void ApplyClothToBones(std::vector<ClothBone>& chain)
    {
        using namespace DirectX;

        for (int i = 1; i < chain.size(); ++i)
        {
            auto& parent = chain[i - 1];
            auto& child = chain[i];

            XMVECTOR p = MathHelper::Load(parent.position);
            XMVECTOR c = MathHelper::Load(child.position);
            XMVECTOR worldDir = XMVector3Normalize(c - p);

            const auto& parentNode = modelNodes[parent.nodeIndex];
            XMMATRIX parentGlobal = XMLoadFloat4x4(&parentNode.globalTransform);

            XMVECTOR parentRot = XMQuaternionRotationMatrix(parentGlobal);
            XMVECTOR parentRotInv = XMQuaternionInverse(parentRot);

            XMVECTOR localDir = XMVector3Rotate(worldDir, parentRotInv);

            XMVECTOR rest = XMVector3Normalize(MathHelper::Load(child.restDir));

            XMVECTOR rot = FromToRotation(rest, localDir);

            XMVECTOR animRot = XMLoadFloat4(&modelNodes[child.nodeIndex].rotation);

            XMVECTOR finalRot = XMQuaternionMultiply(rot, animRot);

            XMStoreFloat4(&modelNodes[child.nodeIndex].rotation, finalRot);
        }
    }
    SkeletalMeshComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :MeshComponent(name, owner)
    {
    }

    void SetModel(const std::string& filename, bool isSaveVerticesData = false, bool convertToLHS = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        //model = std::make_shared<InterleavedGltfModel>(device, filename, ModelTypes::ModelMode::SkeletalMesh, isSaveVerticesData);
        model = AssetManager::Get().LoadModel(device, filename, ModelTypes::ModelMode::SkeletalMesh, isSaveVerticesData, convertToLHS);
        modelNodes = model->GetNodes();
    }


    Transform GetSocketTransform(int socketNode) const override;

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

    void SetModel(const std::string& filename, bool isSaveVerticesData = false, bool convertToLHS = false)override
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
            return model->GetJointWorldPosition(name, modelNodes, parentWorld);
        }
        else
        {
            DirectX::XMFLOAT4X4 world = GetComponentWorldTransform().ToWorldTransform();
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


    void SetModel(const std::string& filename, bool isSaveVerticesData = false, bool convertToLHS = false)override
    {
        ID3D11Device* device = Graphics::GetDevice();
        model = AssetManager::Get().LoadModel(device, filename, ModelTypes::ModelMode::StaticMesh, isSaveVerticesData, convertToLHS);
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





