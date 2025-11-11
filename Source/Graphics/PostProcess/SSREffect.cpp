#include "SSREffect.h"

#include "imgui.h"
#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"


void SSREffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    ssrBuffer = std::make_unique<FrameBuffer>(device, width, height, true);
    ssrCBuffer = std::make_unique<ConstantBuffer<SSRConstantBuffer>>(device);

    HRESULT hr = CreatePsFromCSO(device, "./Shader/SSRPS.cso", ssrPS.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void SSREffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* gBufferPosition, ID3D11ShaderResourceView* shadowMap)
{
    ssrCBuffer->data.reflectionIntensity = reflectionIntensity;
    ssrCBuffer->data.maxDistance = maxDistance;
    ssrCBuffer->data.resolution = resolution;
    ssrCBuffer->data.steps = steps;
    ssrCBuffer->data.thickness = thickness;
    ssrCBuffer->Activate(immediateContext, 5);

    ssrBuffer->Clear(immediateContext, 0, 0, 0, 0);
    ssrBuffer->Activate(immediateContext);

    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
    ID3D11ShaderResourceView* shaderResourceViews[]
    {
        gBufferPosition,        //gBufferPosition
        gbufferNormal,          //gbufferNormal
        gbufferColor,           //gbufferColor
    };
    fullScreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), ssrPS.Get());

    ssrBuffer->Deactivate(immediateContext);
}

void SSREffect::DrawDebugUI()
{
#ifdef USE_IMGUI
    ImGui::Begin("ssr");
    ImGui::SliderFloat("Reflection Intensity", &reflectionIntensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Max Distance", &maxDistance, 0.0f, 30.0f);
    ImGui::SliderFloat("Resolution", &resolution, 0.0f, 1.0f);
    ImGui::SliderInt("Steps", &steps, 0, 20);
    ImGui::SliderFloat("Thickness", &thickness, 0.0f, 1.0f);
    ImGui::End();
#endif
}