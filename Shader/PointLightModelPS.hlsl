#include "GltfModel.hlsli"

GBUFFER_PS_OUT main(VS_OUT pin, bool isFrontFace : SV_IsFrontFace)
{
    GBUFFER_PS_OUT pout;
    float3 emissive = cpuColor.rgb;
    emissive *= emissionPower;
    pout.position = pin.wPosition; // world 空間
    float3 N = normalize(pin.wNormal.xyz);
    pout.gBuffer3Normal = float4(N.xyz, 0); // world 空間
    pout.albedo = float4(1, 1, 1, 1); // 仮。点光源はemissiveで色をつけるからここでは白にしておく
#if 1
    float n =
    sin(pin.wPosition.y * 15 + elapsedTime * 4) *
    sin(pin.wPosition.x * 10 + elapsedTime * 3);

    emissive *= 0.8 + 0.4 * n;

// --- コア表現（改良版） ---
    float2 uv = pin.texcoord;

    // 中心化
    float2 centered = uv - 0.5;
    // 縦に伸ばす
    centered.y *= 1.5;
    //  歪み
    float distort =
    sin(pin.wPosition.y * 10 + elapsedTime * 3) * 0.05;
    centered.x += distort;
    // 距離
    float dist = length(centered);
    //  なめらかコア
    float core = smoothstep(0.6, 0.0, dist);
    //  下だけ強く
    float vertical = 1.0 - uv.y;
    core *= vertical;
    //  中心強調
    core = pow(core, 1.5);
    // 色
    float3 coreColor = float3(1.0, 1.0, 1.0);
    // 合成
    emissive = lerp(emissive, coreColor * emissionPower, core * 0.8);
#endif 
    // 元々wは１だったがスカイマップなどの時に使用するため、２は点光源であることを示すフラグ
    pout.emissive = float4(emissive, 2);
    pout.material = float4(0.0, 0.0, 0.0, 0.0);
    return pout;
}