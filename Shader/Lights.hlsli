// ì_åıåπ
struct PointLights
{
    float4 position;
    float4 color;
    float range;
    float padding[3];
};

struct SpotLights
{
    float4 position;
    float4 direction;
    float4 color;
    float range;
    float innerCorn;
    float outerCorn;
};

cbuffer LIGHT_CONSTANT_BUFFER : register(b11)
{
    float4 lightDirection; // w:rimPower
    float4 colorLight; //w colorPower
    float iblIntensity;
    int directionalLightEnable; // ïΩçsåıåπÇÃ on / off
    int pointLightEnable;
    int pointLightCount;
    float3 rimColor;
    float rimIntensity;
    PointLights pointLights[32];
};
