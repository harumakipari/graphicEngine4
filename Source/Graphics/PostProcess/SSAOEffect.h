#pragma once

#include "FrameBuffer.h"
#include "FullScreenQuad.h"
#include "SceneEffectBase.h"

class SSAOEffect:public SceneEffectBase
{
public:
    struct SSAOConstantBuffer
    {
        float sigma=0.3f;
        float power = 1.0f;
        bool improvedNormalReconstructionFromDepth = 1;
        bool bilateralBlur = true;
    };

public:
    SSAOEffect():SceneEffectBase("SSAOEffect"){}

    ~SSAOEffect() = default;

    // �|�X�g�G�t�F�N�g�����i���\�[�X�쐬�j 
    void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) override;

    void Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal,
        ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* shadowMap) override;

    // �o�́i���̃G�t�F�N�g��ŏI�����ɓn���p�j
    ID3D11ShaderResourceView* GetOutputSRV()const override
    {
        //return fogBuffer->shaderResourceViews[0].Get();
    }

    // UI ���� (ImGui)
    void DrawDebugUI()override;

private:
    std::unique_ptr<FrameBuffer> ssaoBuffer;
    std::unique_ptr<FullScreenQuad> fullScreenQuad;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ssaoPS;

};