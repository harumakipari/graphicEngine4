#pragma once

#include <d3d11.h>
#include <memory>
#include <string>

#include "FrameBuffer.h"

class PostEffectBase
{
public:
    PostEffectBase(std::string className) :name(std::move(className)) {}

    virtual ~PostEffectBase() = default;

    // �|�X�g�G�t�F�N�g�����i���\�[�X�쐬�j 
    virtual void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) = 0;

    // �`�揈���i���́F�O�t���[���̃����_�[�^�[�Q�b�g�j
    virtual void Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* inputSrv) = 0;

    // �o�́i���̃G�t�F�N�g��ŏI�����ɓn���p�j
    virtual ID3D11ShaderResourceView* GetOutputSRV()const = 0;

    virtual const std::string& GetName() const { return name; }

    // UI ���� (ImGui)
    virtual void DrawDebugUI() {}

protected:
    std::unique_ptr<FrameBuffer> outputBuffer;  // �o�͐�
    std::string name = "";
};


