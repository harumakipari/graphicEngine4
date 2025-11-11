#include "FogEffect.h"

#include "imgui.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"

void FogEffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fogCBuffer = std::make_unique<ConstantBuffer<FogConstants>>(device);
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    fogBuffer = std::make_unique<FrameBuffer>(device, width, height, true);
    HRESULT hr = CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", fogPS.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void FogEffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* gBufferPosition, ID3D11ShaderResourceView* shadowMap)
{
    fogCBuffer->Activate(immediateContext, 8);

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
#ifdef USE_IMGUI
    ImGui::ColorEdit3("Fog Color", fogCBuffer->data.fogColor);
    ImGui::SliderFloat("Intensity", &(fogCBuffer->data.fogColor[3]), 0.0f, 10.0f);
    ImGui::SliderFloat("Density", &fogCBuffer->data.fogDensity, 0.0f, 0.05f, "%.6f");
    ImGui::SliderFloat("Height Falloff", &fogCBuffer->data.fogHeightFalloff, 0.001f, 1.0f, "%.4f");
    ImGui::SliderFloat("Cutoff Distance", &fogCBuffer->data.fogCutoffDistance, 0.0f, 1000.0f);
    ImGui::SliderFloat("Ground Level", &fogCBuffer->data.groundLevel, -100.0f, 100.0f);
    ImGui::SliderFloat("Mie Scattering", &fogCBuffer->data.mieScatteringFactor, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Time Scale", &fogCBuffer->data.timeScale, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Noise Scale", &fogCBuffer->data.noiseScale, 0.0f, 0.5f, "%.4f");
#endif
}