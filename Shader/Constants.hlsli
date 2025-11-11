#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    float elapsedTime;
    float deltaTimesss;
    float gravitysss;
}

cbuffer SHADER_CONSTANT_BUFFER : register(b9)
{
    bool enableSSAO;
    bool enableCascadedShadowMaps;
    bool enableSSR;
    bool enableFog;

    bool enableBloom;
    bool enableBlurss;
    bool directionalLightEnablesss = true; // ïΩçsåıåπÇÃ on / off
    bool colorizeCascadedLayer = false;

    float shadowColor = 0.2f;
    float shadowDepthBias = 0.0005f;
}
// CASCADED_SHADOW_MAPS
cbuffer CSM_CONSTANTS : register(b3)
{
    row_major float4x4 cascadedMatrices[4];
    float4 cascadedPlaneDistances;
}
// FOG
cbuffer FOG_CONSTANTS_BUFFER : register(b8)
{
    float4 fogColor;
    
    float fogDensity;
    float fogHeightFalloff;
    float groundLevel;
    float fogCutoffDistance;
    
    float mieScatteringCoef;
    
    bool enableDither;
    bool enableBlur;
    
    float timeScale;
    float noiseScale;
}

cbuffer VOLUMETRIC_CLOUDSCAPES_CONSTANT_BUFFER : register(b5)
{
    float2 windDirection;
    float2 cloudAltitudesMinMax; // highest and lowest altitudes at which clouds are distributed
    
    float windSpeed; // [0.0, 20.0]
    
    float densityScale; // [0.01,0.2]
    float cloudCoverageScale; // [0.1,1.0]
    float rainCloudAbsorptionScale;
    float cloudTypeScale;
    
    float earthRadius; // earth radius
    float horizonDistanceScale;
    float lowFreqyentlyPerlinWorleySamplingScale;
    float highFreqyentlyWorleySamplingScale;
    float cloudDensityLongDistanceScale;
    bool enablePowderedSugarEffect;
    
    uint rayMarchingSteps;
    bool autoRayMarchingSteps;
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

#endif