#include "GltfModel.hlsli"

// 2次ベジェ曲線の計算
float3 QuadricBezier(float3 p0, float3 p1, float3 p2, float t)
{
    float u = 1.0 - t;
    return u * u * p0 + 2 * u * t * p1 + t * t * p2;
}

// 2次ベジェ曲線の接線
float3 QuadricBezierTangent(float3 p0, float3 p1, float3 p2, float t)
{
    return 2 * ((1 - t) * (p1 - p0) + t * (p2 - p1));
}

// ロドリゲスの回転公式
float3 RotateVector(float3 v, float3 axis, float angle)
{
    float cosA = cos(angle);
    float sinA = sin(angle);
    return cosA * v + (1 - cosA) * dot(axis, v) * axis + sinA * cross(axis, v);
}

float3 RodriguesRotate(float3 v, float3 axis, float cosA, float sinA)
{
    return cosA * v + sinA * cross(axis, v) + (1.0 - cosA) * dot(axis, v) * axis;
}

cbuffer ELASTIC_CONSTANT_BUFFER : register(b6)
{
    float4 p1; // 始点
    float4 p2; // 制御点
    float4 p3; // 終点
    float buildProgress; // 0.0 ~ 1.0  t
    float buildHeight; // ビルの高さ
}

float3 SafeNormalize(float3 v, float3 fallback)
{
    float len = length(v);
    if (len < 1e-5)
    {
        return fallback;
    }
    return v / len;
}

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    float sigma = vin.tangent.w;

    // ① モデル→ワールド
    float4 worldPos4 = mul(vin.position, world);
    float3 worldPos = worldPos4.xyz;

    // ② t (高さ比)
    float meshHeight = max(buildHeight, 1e-5);
    float t = saturate((worldPos.y - p1.y) / meshHeight);

    // ③ ベジェ点 & 接線（ワールド空間）
    float3 bezierPos = QuadricBezier(p1.xyz, p2.xyz, p3.xyz, t);
    float3 bezierTangentRaw = QuadricBezierTangent(p1.xyz, p2.xyz, p3.xyz, t);
    float3 bezierTangent = SafeNormalize(bezierTangentRaw, float3(0, 0, 1));

    // ④ 直線の芯とローカルオフセット
    float3 straightPos = p1.xyz + float3(0, (worldPos.y - p1.y), 0);
    float3 localOffset = worldPos - straightPos;

    // ⑤ 回転軸・角度の準備（安定処理）
    float3 up = float3(0, 1, 0);
    float3 rotAxisRaw = cross(up, bezierTangent);
    float axisLen = length(rotAxisRaw);

    // デフォルト：回転しない (axisLen が小さい = up と接線が平行)
    float3 rotatedOffset = localOffset;
    float3 outNormal;
    float3 outTangent;

    if (axisLen > 1e-5)
    {
        // 正規化
        float3 rotAxis = rotAxisRaw / axisLen;
#if 1
        float cosAngle = clamp(dot(up, bezierTangent), -1.0, 1.0);
        float angle = acos(cosAngle);
        float maxAngle = radians(80.0); // 30 度以上は回転しない
        angle = clamp(angle, -maxAngle, maxAngle);
        float sinAngle = sin(angle);
        //float sinAngle = sqrt(max(0.0, 1.0 - cosAngle * cosAngle));

#else
        float angle = atan2(axisLen, clamp(dot(up, bezierTangent), -1.0, 1.0));
        float maxAngleDeg = 30.0; // 30 度以上は回転しない
        float maxAngle = maxAngleDeg * 3.14159265 / 180.0;
        
        float useAngle = clamp(angle, -maxAngle, maxAngle);
        float cosAngle = cos(useAngle);
        float sinAngle = sin(useAngle);

#endif
        // Rodriguesでローカルオフセットを回転
        rotatedOffset = RodriguesRotate(localOffset, rotAxis, cosAngle, sinAngle);
        
        // 法線／接線も同じ回転を適用する（まずワールドに変換しておく）
        outNormal = normalize(mul(vin.normal.xyz, (float3x3) world));
        outTangent = normalize(mul(vin.tangent.xyz, (float3x3) world));
        //outNormal = normalize(mul((float3x3) world, vin.normal.xyz));
        //outTangent = normalize(mul((float3x3) world, vin.tangent.xyz));

        outNormal = RodriguesRotate(outNormal, rotAxis, cosAngle, sinAngle);
        outTangent = RodriguesRotate(outTangent, rotAxis, cosAngle, sinAngle);
    }
    else
    {
        // 回転不要: 法線／接線をワールド変換だけしておく
        outNormal = normalize(mul((float3x3) world, vin.normal.xyz));
        outTangent = normalize(mul((float3x3) world, vin.tangent.xyz));
    }

    // ⑥ 変形頂点
    float3 deformedPos = bezierPos + rotatedOffset;

    // ⑦ 出力
    vout.wPosition = float4(deformedPos, 1.0f);
    vout.position = mul(vout.wPosition, vviewProjection);

    vout.wNormal = float4(SafeNormalize(outNormal, float3(0, 1, 0)), 0.0f);
    vout.wTangent = float4(SafeNormalize(outTangent, float3(1, 0, 0)), sigma);

    vout.texcoord = vin.texcoord;
    return vout;
    // モデル座標系 -> ワールド座標系
    //float4 worldPos = mul(vin.position, world);
    
    // 高さ比でtを決定
    float localY = worldPos.y;
    t = saturate((localY - p1.y) / max(buildHeight, 0.00001f));
    
    // ベジェ曲線上の位置を計算　ワールド空間
    bezierPos = QuadricBezier(p1.xyz, p2.xyz, p3.xyz, t);
    
    //float3 straightPos = p1.xyz + float3(0, (localY - p1.y), 0);

    //float3 deformedPos = bezierPos + (worldPos.xyz - straightPos);

    
    vout.position = mul(float4(bezierPos, 1), vviewProjection);
    //vout.wPosition = float4(deformedPos, 1);
    vout.wPosition = float4(bezierPos, 1);

    vout.wNormal = vout.wNormal = normalize(mul(vin.normal, world));
    float3 worldTangent = normalize(mul(float4(vin.tangent.xyz, 0), world).xyz);
    vout.wTangent = float4(worldTangent, vin.tangent.w);

    vout.texcoord = vin.texcoord;
    return vout;
    
    // p1,p2,p3 のベジェ曲線を作るから。。p1 は頂点の座標？
    //float4 p1 = vin.position;
    
    float3 base = p1.xyz;
    meshHeight = p3.y - p1.y;
    float3 meshUp = float3(0, meshHeight, 0);
    float3 meshTop = base + meshUp;
    
    float3 curveDir = normalize(QuadricBezierTangent(p1.xyz, p2.xyz, p3.xyz, buildProgress)).xyz;
    float3 curveTarget = p2.xyz;
    float3 curveEnd = p3.xyz;
    
    float height = clamp((vin.position.y - base.y) / meshHeight, 0.0, 1.0);
    //float3 bezierPos = QuadricBezier(p1.xyz, p2.xyz, p3.xyz, height);
    
    float4 newPos = float4(vin.position.x, 0, vin.position.z, 1);
    //newPos.xyz += bezierPos - base;
    
    vout.position = newPos;
    
    
        
    //vin.position.w = 1;
    //vout.position = mul(vin.position, mul(world, viewProjection));
    
    ////vout.position /= vout.position.w; // to ndc
    
    
    //vout.wPosition = mul(vin.position, world);
    
    //vin.normal.w = 0;
    //vout.wNormal = normalize(mul(vin.normal, world));
    //vin.tangent.w = 0;
    //vout.wTangent = normalize(mul(vin.tangent, world));
    //vout.wTangent.w = sigma;
    
    //vout.texcoord = vin.texcoord;
    
    //return vout;

    
    vin.position.w = 1;
    vout.position = mul(newPos, mul(world, vviewProjection));

    vout.wPosition = mul(newPos, world);
    
    vin.normal.w = 0;
    vout.wNormal = normalize(mul(vin.normal, world));
    
    vin.tangent.w = 0;
    vout.wTangent = normalize(mul(vin.tangent, world));
    vout.wTangent.w = sigma;
    
    vout.texcoord = vin.texcoord;
    
    return vout;
}