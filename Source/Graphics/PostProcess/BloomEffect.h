#pragma once

#include <memory>

#include "PostEffectBase.h"
#include "FullScreenQuad.h"
#include "Graphics/Core/ConstantBuffer.h"

class BloomEffect :public PostEffectBase
{
public:
    BloomEffect() = default;
	~BloomEffect() = default;

    // �|�X�g�G�t�F�N�g�����i���\�[�X�쐬�j 
    void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) override;

    // �`�揈���i���́F�O�t���[���̃����_�[�^�[�Q�b�g�j
    void Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* inputSrv)override;

    // �o�́i���̃G�t�F�N�g��ŏI�����ɓn���p�j
    ID3D11ShaderResourceView* GetOutputSRV()const override
    {
		return glowExtraction->shaderResourceViews[0].Get();
    }

    // UI ���� (ImGui)
	void DrawDebugUI()override;

private:
public:
	//float bloom_extraction_threshold = 0.85f;
	//float bloom_intensity = 0.016f;
	float bloomExtractionThreshold = 0.777f;
	float bloomIntensity = 0.163f;

private:
	std::unique_ptr<FullScreenQuad> bitBlockTransfer;
	std::unique_ptr<FrameBuffer> glowExtraction;

	static constexpr size_t downsampledCount = 6;
	std::unique_ptr<FrameBuffer> gaussianBlur[downsampledCount][2];

	Microsoft::WRL::ComPtr<ID3D11PixelShader> glowExtractionPs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussianBlurDownsamplingPs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussianBlurHorizontalPs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussianBlurVerticalPs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussianBlurUpsamplingPs;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	struct BloomConstants
	{
		float bloomExtractionThreshold;
		float bloomIntensity;
		float something[2];
	};

	std::unique_ptr<ConstantBuffer<BloomConstants>> bloomConstant;
};

