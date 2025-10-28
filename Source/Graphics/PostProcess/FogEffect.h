#pragma once

#include <memory>

#include "PostEffectBase.h"
#include "FullScreenQuad.h"
#include "Graphics/Core/ConstantBuffer.h"

class FogEffect:public PostEffectBase
{
public:
    struct FogConstants
    {
        float fogColor[4] = { 1.000f,1.000f, 1.000f, 1.000f }; // w: fog intensuty

        float fogDensity = 0.02f;
        float fogHeightFalloff = 0.05f;
        float groundLevel = 0.0f;
        float fogCutoffDistance = 500.0f;

        float mieScatteringFactor = 0.55f;    

        int enableDither = 1;
        int enableBlur = 1;

        float timeScale = 0.35f;
        float noiseScale = 0.2f;
        float pads[3];
    };

public:
    FogEffect() = default;
    ~FogEffect() = default;

    // ポストエフェクト生成（リソース作成） 
    void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) override;

    // 描画処理（入力：前フレームのレンダーターゲット）
    void Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* inputSrv)override;

    // 出力（次のエフェクトや最終合成に渡す用）
    ID3D11ShaderResourceView* GetOutputSRV()const override
    {
        return fogSRV.Get();
        //return glowExtraction->shaderResourceViews[0].Get();
    }

    // UI 調整 (ImGui)
    void DrawDebugUI()override {}

private:
    Microsoft::WRL::ComPtr<ID3D11PixelShader> fogPS;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fogSRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> fogRTV;
};
