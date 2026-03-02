#include "pch.h"
#include "FogEffect.h"

#include <DDSTextureLoader.h>

#include "imgui.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Resource/Texture.h"

void FogEffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fogCBuffer = std::make_unique<ConstantBuffer<FogConstants>>(device);
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    fogBuffer = std::make_unique<FrameBuffer>(device, width / 2, height / 2, false, DXGI_FORMAT_R16_FLOAT);
    //fogBuffer = std::make_unique<FrameBuffer>(device, width / 2, height / 2, false);
    HRESULT hr = CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", fogPS.ReleaseAndGetAddressOf());
    //HRESULT hr = CreatePsFromCSO(device, "./Shader/VolumetricLightPS.cso", fogPS.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    D3D11_TEXTURE2D_DESC texture2dDesc;
#if 0
    hr = LoadTextureFromFile(device, L"./Data/ShaderTextures/noise.png", noise2d.GetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#else
    hr = LoadTextureFromFile(device, L"./Data/ShaderTextures/noise1.png", noise2d.ReleaseAndGetAddressOf(), &texture2dDesc);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#endif

#if 1
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    hr = DirectX::CreateDDSTextureFromFile(device, L"./Data/ShaderTextures/_noise_3d.dds", resource.ReleaseAndGetAddressOf(), noise3d.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
#else
    ////////PreComputeNoiseTexture3d(device.Get(), 64, noise3d.GetAddressOf());
#endif
}

void FogEffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* gBufferPosition, ID3D11ShaderResourceView* gBufferPbrValue, ID3D11ShaderResourceView* shadowMap)
{
    fogCBuffer->data.enableDither = enableDither;
    fogCBuffer->Activate(immediateContext, 8);

    fogBuffer->Clear(immediateContext, 0, 0, 0, 1);
    fogBuffer->Activate(immediateContext);

    immediateContext->PSSetShaderResources(30, 1, noise2d.GetAddressOf());
    immediateContext->PSSetShaderResources(31, 1, noise3d.GetAddressOf());


    RenderState::BindBlendState(immediateContext, BLEND_STATE::NONE);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_OFF_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERIZE_STATE::SOLID_CULL_NONE);
    ID3D11ShaderResourceView* shaderResourceViews[]
    {
        gbufferDepth,       //depthMap
        gBufferPosition,    //worldPosition
        shadowMap,          //cascadedShadowMaps
    };
    fullScreenQuad->Blit(immediateContext, shaderResourceViews, 0, _countof(shaderResourceViews), fogPS.Get());

    fogBuffer->Deactivate(immediateContext);


}

void FogEffect::DrawDebugUI()
{
#ifdef USE_IMGUI
    ImGui::Checkbox("enable", &enabled);
    ImGui::ColorEdit4("Fog Color", fogCBuffer->data.fogColor);
    ImGui::SliderFloat("Intensity", &(fogCBuffer->data.fogColor[3]), 0.0f, 10.0f);
    ImGui::SliderFloat("Density", &fogCBuffer->data.fogDensity, 0.0f, 10.0f, "%.6f");
    ImGui::SliderFloat("Height Falloff", &fogCBuffer->data.fogHeightFalloff, 0.001f, 1.0f, "%.4f");
    ImGui::SliderFloat("Cutoff Distance", &fogCBuffer->data.fogCutoffDistance, 0.0f, 1000.0f);
    ImGui::SliderFloat("Ground Level", &fogCBuffer->data.groundLevel, -100.0f, 100.0f);
    ImGui::SliderFloat("Mie Scattering", &fogCBuffer->data.mieScatteringFactor, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Time Scale", &fogCBuffer->data.timeScale, 0.0f, 1.0f, "%.4f");
    ImGui::SliderFloat("Noise Scale", &fogCBuffer->data.noiseScale, 0.0f, 0.5f, "%.4f");
    ImGui::Checkbox("Enable Dither", &enableDither);
#endif
}