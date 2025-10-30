#include "SSAOEffect.h"

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"

void SSAOEffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    ssaoBuffer = std::make_unique<FrameBuffer>(device, width, height, true);
    HRESULT hr = CreatePsFromCSO(device, "./Shader/SSAOPS.cso", ssaoPS.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void SSAOEffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* shadowMap)
{
    ssaoBuffer->Clear(immediateContext, 0, 0, 0, 0);
    ssaoBuffer->Activate(immediateContext);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    ID3D11ShaderResourceView* shaderResourceViews[]
    {
        gbufferDepth,       //depthMap
    };
    fullScreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), ssaoPS.Get());

    ssaoBuffer->Deactivate(immediateContext);
}

