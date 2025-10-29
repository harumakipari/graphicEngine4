#include "SSAOEffect.h"

#include "Engine/Utility/Win32Utils.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Core/Shader.h"

void SSAOEffect::Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
{
    fullScreenQuad = std::make_unique<FullScreenQuad>(device);
    ssaoBuffer = std::make_unique<FrameBuffer>(device, width, height, true);
    HRESULT hr = CreatePsFromCSO(device, "./Shader/VolumetricFogPS.cso", ssaoPS.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void SSAOEffect::Apply(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal, ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* shadowMap)
{
    
}

