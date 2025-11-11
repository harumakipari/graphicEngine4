struct ClothVertex
{
    float4 position;
    float4 normal;
    float4 tangent;
    float4 texcoord;
    
    float4 oldVelocity;
    float4 velocity;
    float4 newPosition;
};

struct ClothEdge
{
    uint id;
    float restLength;
    float3 delta;
};

StructuredBuffer<ClothVertex> clothVerticesSRV : register(t0);
StructuredBuffer<ClothEdge> clothEdges : register(t1);
RWStructuredBuffer<ClothVertex> clothVerticesUAV : register(u0);

#define CLOTH_EDGES_PER_VERTEX 4
#define SPRING_FORCE 20000.0
#define MASS 0.1
#define GRAVITY 0.98
#define DAMPING 0.04

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    float elapsedTime;
    float deltaTime;
    float gravity;
}

float4x4 RightMultMatrix(float4 a)
{
    return float4x4
    (
    float4(a.w, -a.z, a.y, -a.x),
    float4(a.z, a.w, -a.x, -a.y),
    float4(-a.y, a.x, a.w, -a.z),
    float4(a.x, a.y, a.z, a.w)
    );
}

float4x4 LeftMultMatrix(float4 b)
{
    return float4x4
    (
        float4(b.w, b.z, -b.y, -b.x),
        float4(-b.z, b.w, b.x, -b.y),
        float4(b.y, -b.x, b.w, -b.z),
        float4(b.x, b.y, b.z, b.w)
    );
}

#define NUM_THREAD_X 16

[numthreads(NUM_THREAD_X, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
    float4x4 errorMatrix = 0; // 全要素 0 初期化

    float3 currentPosition = clothVerticesSRV[id.x].newPosition.xyz;

    for (uint i = 0; i < CLOTH_EDGES_PER_VERTEX; i++)
    {
        ClothEdge edge = clothEdges[id.x * CLOTH_EDGES_PER_VERTEX + i];
        if (edge.id != 0xffffffff)
        {
            float3 delta = edge.delta.xyz; // rest pose のエッジ
            float3 currentDelta = clothVerticesSRV[edge.id].newPosition.xyz - currentPosition;

            float4x4 m = RightMultMatrix(float4(delta, 0.0))
                       - LeftMultMatrix(float4(currentDelta, 0.0));

            errorMatrix += mul(transpose(m), m);
        }
    }

    //float4 currentRotation = 
    //float4 rotationGrad = mul(currentRotation, errorMatrix) * 2.0;
    //float4 newRotation = normalize(currentRotation - rotationGrad * 0.25);

    //SetRotation(id.x, newRotation);

    //clothVertices[id.x].oldVelocity = clothVertices[id.x].velocity;
    //SetPosition(id.x, clothVertices[id.x].newPosition);
    
    //currentClothVertices[id.x].position = preClothVerticesSRV[id.x].newPosition;
    clothVerticesUAV[id.x].position = clothVerticesUAV[id.x].position;
} 