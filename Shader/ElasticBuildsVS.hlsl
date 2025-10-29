#include "GltfModel.hlsli"

// 2���x�W�F�Ȑ��̌v�Z
float3 QuadricBezier(float3 p0, float3 p1, float3 p2, float t)
{
    float u = 1.0 - t;
    return u * u * p0 + 2 * u * t * p1 + t * t * p2;
}

// 2���x�W�F�Ȑ��̐ڐ�
float3 QuadricBezierTangent(float3 p0, float3 p1, float3 p2, float t)
{
    return 2 * ((1 - t) * (p1 - p0) + t * (p2 - p1));
}

// ���h���Q�X�̉�]����
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
    float4 p1; // �n�_
    float4 p2; // ����_
    float4 p3; // �I�_
    float buildProgress; // 0.0 ~ 1.0  t
    float buildHeight; // �r���̍���
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

    // �@ ���f�������[���h
    float4 worldPos4 = mul(vin.position, world);
    float3 worldPos = worldPos4.xyz;

    // �A t (������)
    float meshHeight = max(buildHeight, 1e-5);
    float t = saturate((worldPos.y - p1.y) / meshHeight);

    // �B �x�W�F�_ & �ڐ��i���[���h��ԁj
    float3 bezierPos = QuadricBezier(p1.xyz, p2.xyz, p3.xyz, t);
    float3 bezierTangentRaw = QuadricBezierTangent(p1.xyz, p2.xyz, p3.xyz, t);
    float3 bezierTangent = SafeNormalize(bezierTangentRaw, float3(0, 0, 1));

    // �C �����̐c�ƃ��[�J���I�t�Z�b�g
    float3 straightPos = p1.xyz + float3(0, (worldPos.y - p1.y), 0);
    float3 localOffset = worldPos - straightPos;

    // �D ��]���E�p�x�̏����i���菈���j
    float3 up = float3(0, 1, 0);
    float3 rotAxisRaw = cross(up, bezierTangent);
    float axisLen = length(rotAxisRaw);

    // �f�t�H���g�F��]���Ȃ� (axisLen �������� = up �Ɛڐ������s)
    float3 rotatedOffset = localOffset;
    float3 outNormal;
    float3 outTangent;

    if (axisLen > 1e-5)
    {
        // ���K��
        float3 rotAxis = rotAxisRaw / axisLen;
#if 1
        float cosAngle = clamp(dot(up, bezierTangent), -1.0, 1.0);
        float angle = acos(cosAngle);
        float maxAngle = radians(80.0); // 30 �x�ȏ�͉�]���Ȃ�
        angle = clamp(angle, -maxAngle, maxAngle);
        float sinAngle = sin(angle);
        //float sinAngle = sqrt(max(0.0, 1.0 - cosAngle * cosAngle));

#else
        float angle = atan2(axisLen, clamp(dot(up, bezierTangent), -1.0, 1.0));
        float maxAngleDeg = 30.0; // 30 �x�ȏ�͉�]���Ȃ�
        float maxAngle = maxAngleDeg * 3.14159265 / 180.0;
        
        float useAngle = clamp(angle, -maxAngle, maxAngle);
        float cosAngle = cos(useAngle);
        float sinAngle = sin(useAngle);

#endif
        // Rodrigues�Ń��[�J���I�t�Z�b�g����]
        rotatedOffset = RodriguesRotate(localOffset, rotAxis, cosAngle, sinAngle);
        
        // �@���^�ڐ���������]��K�p����i�܂����[���h�ɕϊ����Ă����j
        outNormal = normalize(mul(vin.normal.xyz, (float3x3) world));
        outTangent = normalize(mul(vin.tangent.xyz, (float3x3) world));
        //outNormal = normalize(mul((float3x3) world, vin.normal.xyz));
        //outTangent = normalize(mul((float3x3) world, vin.tangent.xyz));

        outNormal = RodriguesRotate(outNormal, rotAxis, cosAngle, sinAngle);
        outTangent = RodriguesRotate(outTangent, rotAxis, cosAngle, sinAngle);
    }
    else
    {
        // ��]�s�v: �@���^�ڐ������[���h�ϊ��������Ă���
        outNormal = normalize(mul((float3x3) world, vin.normal.xyz));
        outTangent = normalize(mul((float3x3) world, vin.tangent.xyz));
    }

    // �E �ό`���_
    float3 deformedPos = bezierPos + rotatedOffset;

    // �F �o��
    vout.wPosition = float4(deformedPos, 1.0f);
    vout.position = mul(vout.wPosition, vviewProjection);

    vout.wNormal = float4(SafeNormalize(outNormal, float3(0, 1, 0)), 0.0f);
    vout.wTangent = float4(SafeNormalize(outTangent, float3(1, 0, 0)), sigma);

    vout.texcoord = vin.texcoord;
    return vout;
    // ���f�����W�n -> ���[���h���W�n
    //float4 worldPos = mul(vin.position, world);
    
    // �������t������
    float localY = worldPos.y;
    t = saturate((localY - p1.y) / max(buildHeight, 0.00001f));
    
    // �x�W�F�Ȑ���̈ʒu���v�Z�@���[���h���
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
    
    // p1,p2,p3 �̃x�W�F�Ȑ�����邩��B�Bp1 �͒��_�̍��W�H
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