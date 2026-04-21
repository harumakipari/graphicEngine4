#include "Trail.hlsli"

float4 main(VS_OUT input) : SV_TARGET
{
    return float4(1, 1, 1, input.alpha);
}