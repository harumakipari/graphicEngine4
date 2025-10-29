struct ClothVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 texcoord;
    float3 velocity;
    float3 oldPosition;
    float3 oldVelocity;
    float4 rotation;
    uint isPinned;
};

struct ClothEdge
{
    uint neighbor; // �אڒ��_�̃C���f�b�N�X
    float3 delta; // �����̑��΃x�N�g��
    float restLength; // ��������
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    
    float4 cpuColor;
    
    int material;
    bool hasTangent;
    int skin;
    float dissolveValue; //�f�B�]���u�p
    
    float emission;
    float3 pads;
    
    row_major float4x4 invWorld;
}


StructuredBuffer<ClothVertex> inVertices : register(t0);
StructuredBuffer<ClothEdge> clothEdgeSRV : register(t1);
RWStructuredBuffer<ClothVertex> outVertices : register(u0);

//#define CLOTH_EDGES_PER_VERTEX 4
#define MAX_EDGES 8
#define SPRING_FORCE 20000.0
//#define MASS 0.5
#define MASS 0.5
#define GRAVITY 0.98
#define DAMPING 0.04

// �萔
#define SPRING_STIFFNESS 40.0   // �΂˒萔 40-200
#define SPRING_DAMPING   10.0     // �΂˔S���i���Α��x�ɑ΂��錸���j
#define GLOBAL_DAMPING   0.05     // �S�̎w������

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    bool enableSSAO;
    float reflectionIntensity;
    float time;
    // shader �̃t���O
    bool enableCascadedShadowMaps;
    
    bool enableSSR;
    bool enableFog;
    bool enableBloom;
    float deltaTime;
    
    float gravity;
    float3 aaa;
    
    //row_major float4x4 clothInvWorld;
}

cbuffer VIEW_CONSTANTS_BUFFER : register(b4)
{
    row_major float4x4 viewProjection;
    float4 cameraPositon;
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 inverseProjection;
    row_major float4x4 inverseViewProjection;
    row_major float4x4 invView;
}


cbuffer CLOTH_SIMULATE_CBUFFER : register(b10)
{
    float g;
    uint vertexCount;
    float pading[2];
};

cbuffer SPHERE_CBUFFER : register(b7)
{
    float3 worldPos;
    float radius;
    row_major float4x4 sphereWorld;
    row_major float4x4 sphereInvWorld;
}

void CollideSphere(inout ClothVertex v)
{
 // ���̃��[���h���
    float3 sphereCenterWorld = worldPos;
    float sphereRadiusWorld = radius;
    float friction = 0.4;
    float restitution = 0.1;

    // ���[���h�����[�J���ϊ�
    float4 centerLocal4 = mul(float4(sphereCenterWorld, 1.0), invWorld);
    float3 sphereCenterLocal = centerLocal4.xyz;

    // ���[�J���ł̉����o������
    float3 dir = v.position - sphereCenterLocal;
    float dist = length(dir);

    if (dist < sphereRadiusWorld)
    {
        float3 n = normalize(dir);
        float penetration = sphereRadiusWorld - dist;
        
        // ���艻�t�������o��
        v.position += n * penetration * 1.0;
        //v.position += n * penetration * 0.8;

        // ���x�␳
        float vn = dot(v.velocity, n);
        if (vn < 0.0f)
        {
            float3 velNormal = vn * n;
            float3 velTangent = v.velocity - velNormal;

            // restitution��friction��K�p
            //v.velocity = velTangent * (1.0 - friction) - velNormal * restitution;
            v.velocity = velTangent * 0.9 - velNormal * 0.5;
        }
    }
}

struct Plane
{
    float3 normal;
    float d;
};

cbuffer PLANE_CBUFFER : register(b6)
{
    Plane planes[4];
}

void CollideFloor(inout ClothVertex v, Plane p)
{
    //Plane worldFloor = { float3(1, 1, 0), 1 }; // y=0 �̏�
    
    //Plane worldFloor = { float3(0, 1, 0), 0 }; // y=0 �̏�
    Plane worldFloor = p; // y=0 �̏�
    
    float3 localNormal = mul(float4(worldFloor.normal, 0.0f), invWorld).xyz;
    localNormal = normalize(localNormal);

    // ���ʏ��1�_�����߂�i���[���h��ԂŁj
    float3 worldPoint = worldFloor.normal * worldFloor.d;

    // ��������[�J����Ԃɕϊ�
    float3 localPoint = mul(float4(worldPoint, 1.0f), invWorld).xyz;

    // �z�ƕ��ʂ̋��������߂�
    float dist = dot(localNormal, v.position - localPoint);

    if (dist < 0.0f)
    {
        // �����o��
        v.position -= localNormal * dist * 0.8f;

        // ���x�␳
        float vn = dot(v.velocity, localNormal);
        if (vn < 0.0f)
        {
            float3 velNormal = vn * localNormal;
            float3 velTangent = v.velocity - velNormal;
            float friction = 0.4f;
            float restitution = 0.1f;

            v.velocity = velTangent * (1.0f - friction) - velNormal * restitution;
        }
    }

}

#define NUM_THREAD_X 16

[numthreads(NUM_THREAD_X, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (vertexCount <= id.x)
    {
        return;
    }
    ClothVertex inVertex = inVertices[id.x];
    ClothVertex outVertex = inVertices[id.x];
    
    outVertex = inVertex;
    
    float3 currentPos = inVertex.position;
    
    float3 force = float3(0, 0, 0);
    float3 avgVelocity = float3(0, 0, 0);
    uint edgeCount = 0;
    
    
    if (outVertex.isPinned == 1)
    { // �Œ�_��������
        outVertex.position = inVertex.position;
        outVertex.velocity = float3(0.0, 0.0, 0.0);
        outVertex.oldPosition = inVertex.position;
        outVertex.oldVelocity = inVertex.velocity;
        outVertices[id.x] = outVertex;
        return;
    }
    
    outVertex.oldPosition = inVertex.position; // �O�t���[���̏��
    outVertex.oldVelocity = inVertex.velocity;

    uint baseEdge = id.x * MAX_EDGES; // 8����
    
    for (uint i = 0; i < MAX_EDGES; ++i)
    {
        ClothEdge e = clothEdgeSRV[baseEdge + i];
        if (e.neighbor == 0xffffffffu)
        { // �ߖT�_���Ȃ�������
            continue;
        }
        float3 neighborPos = inVertices[e.neighbor].position;
        float3 delta = neighborPos - currentPos;
        float distance = length(delta);
        if (distance > 1e-6f)
        {
            // �����@���@f �����@x�@(����-���R��)
            float springDelta = distance - e.restLength;
            float3 n = normalize(delta);
            // �t�b�N�̖@���O�O
            float springStiffness = SPRING_STIFFNESS; // �傫���قǍd���Ȃ� ����
            force += n * springStiffness * springDelta;
            
            // �ߖT���x�̕���
            avgVelocity += inVertices[e.neighbor].oldVelocity;
            
            edgeCount++;
        }
    }

    if (edgeCount > 0)
    {
        // �^�������� �d�� �~ ����
        //float4 forceWorld = float4(0, 0, 0, 0);
        float4 forceWorld = float4(0, -9.8 * MASS, 0, 0);
        // �d�͂����f����Ԃɕϊ�����
        float4 forceLocal4 = mul(forceWorld, invWorld);
        force += forceLocal4.xyz;
        float3 currentVelocity = inVertex.velocity;
#if 0
        // ���̉e��
        float4 windDirWorld = normalize(float4(0, 0, 1, 0)); // Z������
        float4 windDirLocal4 = mul(windDirWorld, invWorld);
        float windBase = 5.0f;
        float windVariation = 15.0f;

        // ���Ԃƈʒu�ɂ���炬
        float w = sin(time * 2.0f + id.x * 0.1f) * 0.5f + 0.5f;
        float strength = windBase + windVariation * w;

        // �Ǐ������i������Ƃ�炮�j
        float3 windVel = windDirLocal4 * strength;

        // ���Α��x�R��
        float3 relativeVel = windVel - currentVelocity;
        float dragCoef = 0.2f;
        float3 windForce = dragCoef * relativeVel;
        
        force += windForce;
#endif
        avgVelocity /= float(edgeCount);
        
        // ������
        const float damping = SPRING_DAMPING; // �΂˂̔S��
        float3 newVelocity = currentVelocity + (force / MASS) * deltaTime /** exp(-damping * deltaTime)*/;
        newVelocity *= (1.0f - GLOBAL_DAMPING);
        
        const float smmothing = 5.0;
        float3 smoothedVelocity = lerp(avgVelocity, newVelocity, exp(-smmothing * deltaTime));
        
        
        outVertex.velocity = smoothedVelocity;
        outVertex.position = currentPos + smoothedVelocity * deltaTime;
        
    }
    
    CollideSphere(outVertex);
    for (uint j = 0; j < 2; j++)
    {
        CollideFloor(outVertex, planes[j]);
    }

    outVertices[id.x] = outVertex;
}