// ì_åıåπ
struct PointLights
{
    float4 position;
    float4 color;
    float range;
    float3 paddings;
};


cbuffer LIGHT_CONSTANT_BUFFER : register(b11)
{
    float4 lightDirection; // w:attenuation Rate
    float4 colorLight; //w colorPower
    float iblIntensity;
    int directionalLightEnable; // ïΩçsåıåπÇÃ on / off
    int pointLightEnable;
    int pointLightCount;

    float3 rimColor;
    float rimIntensity;

    float rimPower;
    float Kc;
    float Kl;
    float Kq;

    float diffuseIntensity;
    float specularIntensity;
    float2 value;

    PointLights pointLights[40];
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
