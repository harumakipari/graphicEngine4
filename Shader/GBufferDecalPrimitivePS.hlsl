#include "GBufferDecalPrimitive.hlsli"
#include "Sampler.hlsli"

//  G-Buffer群
Texture2D<float4> gbuffer_base_color : register(t0);
Texture2D<float4> gbuffer_emissive_color : register(t1);
Texture2D<float4> gbuffer_normal : register(t2);
Texture2D<float4> gbuffer_parameter : register(t3);
Texture2D<float4> gbuffer_position : register(t4);

//  デカールテクスチャ
Texture2D<float4> decal_texture : register(t10);

float4 main(VS_OUT pin) : SV_TARGET
{
    //  スクリーン情報からGBuffer参照用UV座標を算出
    float2 screen_size;
    gbuffer_base_color.GetDimensions(screen_size.x, screen_size.y);
    float2 gbuffer_texcoord = pin.position.xy / screen_size;

    float4 worldPosition = gbuffer_position.Sample(samplerStates[POINT], gbuffer_texcoord);

    //  キューブ基準の空間の座標に変換してデカールのUV座標に変換
    float4 cube_texture_position = mul(float4(worldPosition.xyz, 1), decal_inverse_transform);
    cube_texture_position /= cube_texture_position.w;
    cube_texture_position.x = cube_texture_position.x * +0.5f + 0.5f;
    cube_texture_position.y = cube_texture_position.y * -0.5f + 0.5f;

    float2 decal_texcoord = cube_texture_position.xy;
    float4 color = decal_texture.Sample(samplerStates[LINEAR_BORDER_BLACK], decal_texcoord) * pin.color;
    clip(color.a < 0.5f ? -1 : 1);
    return color;
}
