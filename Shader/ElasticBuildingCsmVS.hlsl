// CASCADED_SHADOW_MAPS
#include "GltfModel.hlsli"

struct CsmConstants
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
};

cbuffer csmConstants : register(b3)
{
    CsmConstants csmData;
}

// CASCADED_SHADOW_MAPS
struct VS_OUT_CSM
{
    float4 position : SV_POSITION;
    uint instanceId : INSTANCEID;
};

struct GS_OUTPUT_CSM
{
    float4 position : SV_POSITION;
    uint renderTargetArrayIndex : SV_RENDERTARGETARRAYINDEX;
};


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


VS_OUT_CSM main(VS_IN vin, uint instanceId : SV_INSTANCEID)
{
    VS_OUT_CSM vout;
    
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
    }
    // �E �ό`���_
    float3 deformedPos = bezierPos + rotatedOffset;

    // �F �o��
    vout.position = mul(float4(deformedPos, 1.0f),  csmData.cascadedMatrices[instanceId]);
    vout.instanceId = instanceId;
    return vout;
}