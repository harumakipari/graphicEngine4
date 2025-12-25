struct VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    uint4 joints[2] : JOINTS;
    float4 weights[2] : WEIGHTS;
};

struct BATCH_VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

struct INSTANCE_VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    row_major float4x4 instance_matrix : INSTANCE_MATRIX;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 wPosition : POSITION;
    float4 wNormal : NORMAL;
    float4 wTangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    
    int material;
    bool hasTangent;
    int skin;
    int padding;
    
    row_major float4x4 inverseTransposeWorld;
}


cbuffer PLUS_ALPHA_CONSTANT_BUFFER : register(b7)
{
    float hueShift; // 色相調整
    float saturation; // 彩度調整
    float brightness; // 明度調整
    float dissolve; // ディゾルブ用
    float4 cpuColor; // 色をCPU側で指定する用　（ダメージ当たったときとか）
    float emissionPower; // 自己発光の強さ
}


//cbuffer SCENE_CONSTANT_BUFFER : register(b1)
//{
//    row_major float4x4 viewProjection;
//    float4 lightDirection;
//    float4 cameraPositon;
//    float4 colorLight; //w colorPower
//    row_major float4x4 view;
//    row_major float4x4 projection;
//    row_major float4x4 inverseProjection;
//    row_major float4x4 inverseViewProjection;
//    float iblIntensity;
//    bool enableSsao;
//    float refrectionIntensity;
//}
#include "Constants.hlsli"

struct TextureInfo
{
    int index;
    int texcoord;
};

struct NormalTextureInfo
{
    int index;
    int texcoord;
    float scale;
};

struct OcclusionTextureInfo
{
    int index;
    int texcoord;
    float strength;
};

struct PbrMetallicRoughness
{
    float4 baseColorFactor;
    TextureInfo basecolorTexture;
    float metallicFactor;
    float roughnessFactor;
    TextureInfo metallicRoughnessTexture;
};

struct MaterialConstants
{
    float3 emissiveFactor; // length 3. default [0, 0, 0]
    int alphaMode; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2 
    float alphaCutoff; // default 0.5
    bool doubleSided; // default false;
    
    PbrMetallicRoughness pbrMetallicRoughness;
    
    NormalTextureInfo normalTexture;
    OcclusionTextureInfo occlusionTexture;
    TextureInfo emissiveTexture;
};

StructuredBuffer<MaterialConstants> materials : register(t0);

//PRIMITIVE_JOINT_CONSTANTS定数バッファを定義
static const uint PRIMITIVE_MAX_JOINTS = 512;
cbuffer PRIMITIVE_JOINT_CONSTANTS : register(b2)
{
    row_major float4x4 jointMatrices[PRIMITIVE_MAX_JOINTS];
}


// MULTIPLE_RENDER_TARGETS
struct PS_OUT
{
    float4 color : SV_TARGET0;
    float4 position : SV_TARGET1;
    float4 normal : SV_TARGET2;
};

struct GBUFFER_PS_OUT
{
    float4 normal : SV_TARGET1;     // world normal  w:未使用
    float4 material : SV_TARGET2;   // x:metallic y:occlusion z:roughness w:occlusionStrength
    float4 color : SV_TARGET3;      
    float4 position : SV_TARGET4;   // world position
    float4 emissive : SV_TARGET5;   // w:何かを書き込んでいたら０にするそれ以外は１　スカイマップなどの時に使用
};