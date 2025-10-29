#include "FogEffect.h"

#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"

void FogEffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fogConstants = std::make_unique<ConstantBuffer<FogConstants>>(device);
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    fogBuffer = std::make_unique<FrameBuffer>(device, width, height, true);
    HRESULT hr = CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", fogPS.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void FogEffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* shadowMap)
{
    fogConstants->Activate(immediateContext, 4);

    fogBuffer->Clear(immediateContext, 0, 0, 0, 0);
    fogBuffer->Activate(immediateContext);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    ID3D11ShaderResourceView* shaderResourceViews[]
    {
        gbufferColor,  
        gbufferDepth,       //depthMap
        shadowMap,          //cascadedShadowMaps
    };
    fullScreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), fogPS.Get());

    fogBuffer->Deactivate(immediateContext);
}

void FogEffect::DrawDebugUI()
{
    
}