#include "GltfModel.hlsli"

GBUFFER_PS_OUT main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace)
{
    GBUFFER_PS_OUT pout;
    float3 emissiveFactor = { 1.0, 1.0, 1.0 };

    pout.position = pin.wPosition; // world 空間
    float3 N = normalize(pin.wNormal.xyz);
    pout.gbuffer1Normal = float4(N.xyz, 0); // world 空間
    pout.gbuffer3Color = float4(1, 0, 0, 1); // 点光源用赤色
    pout.emissive = float4(emissiveFactor, 0); // 元々wは１だったがスカイマップなどの時に使用するため０に変更
    pout.material = float4(0.0, 0.0, 0.0, 0.0);

    return pout;
}