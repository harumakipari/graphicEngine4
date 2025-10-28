#pragma once

#include <d3d11.h>
#include <vector>
#include <memory>

#include "FrameBuffer.h"

class PostEffectBase
{
public:
    virtual ~PostEffectBase() = default;

    // �|�X�g�G�t�F�N�g�����i���\�[�X�쐬�j 
    virtual void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) = 0;

    // �`�揈���i���́F�O�t���[���̃����_�[�^�[�Q�b�g�j
    virtual void Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* inputSrv) = 0;

    // �o�́i���̃G�t�F�N�g��ŏI�����ɓn���p�j
    virtual ID3D11ShaderResourceView* GetOutputSRV()const = 0;

    // UI ���� (ImGui)
    virtual void DrawDebugUI() {}

protected:
    std::unique_ptr<FrameBuffer> outputBuffer;  // �o�͐�
};


