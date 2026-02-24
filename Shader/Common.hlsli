#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__


#define PI 3.141592653
#define FLT_EPSILON 1.192092896e-07

float max3(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

// Using pow often result to a warning like this
// "pow(f, e) will not work for negative f, use abs(f) or conditionally handle negative values if you expect them"
// PositivePow remove this warning when you know the value is positive and avoid inf/NAN.
float positive_pow(float base, float power)
{
    return pow(max(abs(base), float(FLT_EPSILON)), power);
}

float2 positive_pow(float2 base, float2 power)
{
    return pow(max(abs(base), float2(FLT_EPSILON, FLT_EPSILON)), power);
}

float3 positive_pow(float3 base, float3 power)
{
    return pow(max(abs(base), float3(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON)), power);
}

float4 positive_pow(float4 base, float4 power)
{
    return pow(max(abs(base), float4(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON)), power);
}

// Gamma decoding / De-gamma
float3 srgb_to_linear(float3 c)
{
#if USE_VERY_FAST_SRGB
    return c * c;
#elif USE_FAST_SRGB
    return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878);
#else
    float3 lo = c / 12.92;
    float3 hi = positive_pow((c + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
    return (c <= 0.04045) ? lo : hi;
#endif
}
float4 srgb_to_linear(float4 c)
{
    return float4(srgb_to_linear(c.rgb), c.a);
}
// Gamma encoding / Gamma correction
float3 linear_to_srgb(float3 c)
{
#if USE_VERY_FAST_SRGB
    return sqrt(c);
#elif USE_FAST_SRGB
    return max(1.055 * positive_pow(c, 0.416666667) - 0.055, 0.0);
#else
    float3 lo = c * 12.92;
    float3 hi = (positive_pow(c, float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
    return (c <= 0.0031308) ? lo : hi;
#endif
}
float4 linear_to_srgb(float4 c)
{
    return float4(linear_to_srgb(c.rgb), c.a);
}

// Z buffer to linear 0..1 depth
float linear_01_depth(float z, float near, float far)
{
    return 1.0 / ((1 - far / near) * z + (far / near));
}

void near_far_from_projection(in float4x4 projection, out float near, out float far)
{
    near = -projection._43 / projection._33;
    far = -projection._33 / (1 - projection._33) * near;
}

float2 uv_to_ndc(float2 uv)
{
    float2 ndc;
    ndc.x = 2.0 * uv.x - 1.0;
    ndc.y = 1.0 - 2.0 * uv.y;
    return ndc;
}
float4 ndc_to_uv(float4 ndc)
{
    float4 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    uv.z = ndc.z;
    uv.w = ndc.w;
    return uv;
}
float4 view_to_uv(float3 pos, column_major float4x4 projection_transform)
{
    float4 ndc = mul(float4(pos, 1.0), projection_transform);
    ndc /= ndc.w;
    return ndc_to_uv(ndc);
}

// Spheremap Transform
// https://aras-p.info/texts/CompactNormalStorage.html
float2 encode_normal(float3 n)
{
    float p = sqrt(n.z * 8 + 8);
    return float2(n.xy / p + 0.5);
}
float3 decode_normal(float2 enc)
{
    float2 fenc = enc * 4 - 2;
    float f = dot(fenc, fenc);
    float g = sqrt(1 - f / 4);
    float3 n;
    n.xy = fenc * g;
    n.z = 1 - f / 2;
    return n;
}


// ----------------------------------------------------------------------------
// EncodeOctahedralNormal
//
// 入力:
//   n : ビュー空間またはワールド空間における正規化された3D法線ベクトル
//
// 出力:
//   [0,1]の範囲のfloat2（UNORMレンダリングターゲット保存に適した形式）
//
// 手順:
//   1. 正規化（安全対策）
//   2. 単位球を八面体に投影
//   3. 下半球を上半球に折りたたむ
//   4. [-1,1] から [0,1] へ再マッピング
// --------------------------- -------------------------------------------------

float2 EncodeOctahedralNormal(float3 n)
{
    // 入力が正規化されていることを確認する。
    // 補間処理により長さがわずかに歪む可能性があるため必須。
    n = normalize(n);

    // 単位球を八面体に投影する。
    // これはL1ノルムで除算することで球を平坦化する。
    //
    // L1 norm = |x| + |y| + |z|
    //
    // これにより、投影ベクトルが八面体の表面上にあることが保証される。
    n /= (abs(n.x) + abs(n.y) + abs(n.z));

    // 投影されたx成分とy成分を取る。
    float2 enc = n.xy;

    // 下半球（z < 0）にいる場合、
    // 連続性を保つために八面体を折りたたむ。
    //
    // これは下半球を上半球に鏡像写像し、
    // 一対一の2次元写像を保証する。
    if (n.z < 0.0f)
    {
        // x/yを交換し、絶対値を反転させ、符号を保持する。
        enc = (1.0f - abs(enc.yx)) * sign(enc.xy);
    }

    // UNORM格納用に[-1,1]範囲を[0,1]に再マッピングする。
    return enc * 0.5f + 0.5f;
}

// ----------------------------------------------------------------------------
// DecodeOctahedralNormal
//
// 入力:
//   e : [0,1] 範囲の符号付き float2
//
// 出力:
//   正規化された 3D ベクトル
//
// 手順:
//   1. [0,1] → [-1,1] に再マッピング
//   2. Z成分を再構築する
//   3. 必要に応じて下半球を展開する
//   4. 数値誤差を補正するために正規化する
// -----------------------------------------------------------------------------

float3 DecodeOctahedralNormal(float2 e)
{
    // UNORM空間 [0,1] から符号付き空間 [-1,1] への再マッピング
    float2 f = e * 2.0f - 1.0f;

    // Z成分を再構築する。
    //
    // 使用した射影により：
    //    x + y + z = 1   (L1空間において)
    //
    // ここでその関係を逆算する。
    float3 n = float3(f.x, f.y, 1.0f - abs(f.x) - abs(f.y));

    // 再構成されたZが負の場合、
    // 折り畳み領域内にあり、展開する必要がある。
    if (n.z < 0.0f)
    {
        n.xy = (1.0f - abs(n.yx)) * sign(n.xy);
    }

    // 累積した浮動小数点誤差を補正するために正規化する。
    return normalize(n);
}




float sq(float t)
{
    return t * t;
}
float2 sq(float2 t)
{
    return t * t;
}
float3 sq(float3 t)
{
    return t * t;
}
float4 sq(float4 t)
{
    return t * t;
}

float apply_ior_to_roughness(float roughness, float ior)
{
    // Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
    // an IOR of 1.5 results in the default amount of microfacet refraction.
    return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}
float3 rgb_mix(float3 base, float3 layer, float3 rgb_alpha)
{
    float rgb_alpha_max = max(rgb_alpha.r, max(rgb_alpha.g, rgb_alpha.b));
    return (1.0 - rgb_alpha_max) * base + rgb_alpha * layer;
}

#endif // __COMMON_HLSL__
