#pragma once
#include<d3d11.h>
#include <vector>
#include <memory>

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"
#include "Components/CollisionShape/ShapeComponent.h"

#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/Core/PipleLineLibrary.h"
#include "Engine/Camera/CameraConstants.h"


class SceneRenderer
{
public:
    SceneRenderer()
    {
        // �萔�o�b�t�@
        viewBuffer = std::make_unique<ConstantBuffer<ViewConstants>>(Graphics::GetDevice());
        primitiveJointCBuffer = std::make_unique<ConstantBuffer<PrimitiveJointConstants>>(Graphics::GetDevice());
        primitiveCBuffer = std::make_unique<ConstantBuffer<PrimitiveConstants>>(Graphics::GetDevice());

        // �p�C�v���C���X�e�[�g
        pipeLineStateSet = std::make_unique<PipeLineStateSet>();
        pipeLineStateSet->InitStaticMesh(Graphics::GetDevice());
        pipeLineStateSet->InitSkeletalMesh(Graphics::GetDevice());
    }

    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;
    SceneRenderer(SceneRenderer&&) noexcept = delete;
    SceneRenderer& operator=(SceneRenderer&&) noexcept = delete;

    virtual ~SceneRenderer() = default;

    // View�֘A�̒萔�o�b�t�@���X�V����
    void UpdateViewConstants(ID3D11DeviceContext* immediateContext, const ViewConstants& data) const
    {
        viewBuffer->data = data;
        viewBuffer->Activate(immediateContext, 8);
    }

    void RenderOpaque(ID3D11DeviceContext* immediateContext/*, std::vector<std::shared_ptr<Actor>> allActors*/) const;

    void RenderMask(ID3D11DeviceContext* immediateContext) const;

    void RenderBlend(ID3D11DeviceContext* immediateContext) const;

    void CastShadowRender(ID3D11DeviceContext* immediateContext);

    void CastShadow(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass);

    void CastShadowWithStaticBatching(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes);

    void Draw(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass) const;

    void DrawWithStaticBatching(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass) const;

    void DrawCloth(ID3D11DeviceContext* immediateContext, const MeshComponent* meshComponent, const DirectX::XMFLOAT4X4& world, const std::vector<InterleavedGltfModel::Node>& animatedNodes, InterleavedGltfModel::RenderPass pass);
private:
    // �J�����̒萔�o�b�t�@
    std::unique_ptr<ConstantBuffer<ViewConstants>> viewBuffer;

    // �p�C�v���C���X�e�[�g
    std::unique_ptr<PipeLineStateSet> pipeLineStateSet;

    // 
    static constexpr int PRIMITIVE_MAX_JOINTS = 512;
    struct PrimitiveJointConstants
    {
        DirectX::XMFLOAT4X4 matrices[PRIMITIVE_MAX_JOINTS];
    };
    std::unique_ptr<ConstantBuffer<PrimitiveJointConstants>> primitiveJointCBuffer;

    struct PrimitiveConstants
    {
        DirectX::XMFLOAT4X4 world;

        DirectX::XMFLOAT4 color{ 1.0f,1.0f,1.0f,1.0f };

        int material{ -1 };
        int hasTangent{ 0 };
        int skin{ -1 };
        float dissolveFactor = 0.0f;

        float emission = 0.0f;
        float pads[3];
    };
    std::unique_ptr<ConstantBuffer<PrimitiveConstants>> primitiveCBuffer;

public:
    // ����RenderPath
    RenderPath currentRenderPath = RenderPath::Deferred;
};

