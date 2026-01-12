#include "FullScreenQuad.hlsli"
#include "Sampler.hlsli"

Texture2D colorTexture : register(t0);


float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, number_of_level;
    colorTexture.GetDimensions(mipLevel, width, height, number_of_level);

    float4 color = colorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    return color;
}