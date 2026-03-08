//--------------------------------------------
// リムライト
//--------------------------------------------
// N:法線(正規化済み)
// E:視点方向ベクトル(正規化済み)
// L:入射ベクトル(正規化済み)
// C :ライト色
// RimPower : リムライトの強さ
float3 CalcRimLight(float3 N, float3 V, float3 color, float power)
{
    float rim = 1.0 - saturate(dot(N, V));
    rim = pow(rim, power);
    return rim * color;
}