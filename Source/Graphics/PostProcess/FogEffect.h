#pragma once

#include <memory>

#include "PostEffectBase.h"
#include "FullScreenQuad.h"
#include "Graphics/Core/ConstantBuffer.h"

class FogEffect:public PostEffectBase
{
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
