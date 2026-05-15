#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Sampler.hlsli"
#include "FilterFunctions.hlsli"
#include "Lights.hlsli"

Texture2D sceneColorTexture : register(t0);

cbuffer GAME_SCENE_CONSTANT_BUFFER : register(b12)
{
    float2 playerScreenPosition; //プレイヤーの場所　死亡演出に必要な定数バッファ
    float2 screenSize;

    float radius = 0.0f;
    float3 gameOverColor;
};

float4 main(VS_OUT pin) : SV_TARGET
{
    uint mipLevel = 0, width, height, numberOfLevel, levels;
    sceneColorTexture.GetDimensions(mipLevel, width, height, numberOfLevel);

    // シーンからライティング済みのカラーテクスチャ
    float4 sceneColor = sceneColorTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord);
    float4 color = sceneColor;

    float2 uv = pin.texcoord;

    float2 delta = uv - playerScreenPosition;

    float aspectX =
    screenSize.x / screenSize.y;
    delta.x *= aspectX;

    float dist = length(delta);
    float softness = 0.0f;

    float mask = smoothstep(radius, radius + softness, dist);


    color.rgb = lerp(color.rgb, gameOverColor, mask);
    //color.rgb *= (1.0f - mask);

    return color;
}