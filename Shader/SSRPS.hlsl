#include "Sampler.hlsli"
#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"

float2 NdcToUv(float2 ndc)
{
    float2 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    return uv;
}

inline float FSchlick(float f0, float cos)
{
    return f0 + (1 - f0) * pow(1 - cos, 5);
}

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D colorTexture : register(t2);


float3 main(VS_OUT pin) : SV_TARGET
{
    // SCREEN_SPACE_REFLECTION
    int steps = 10;
    
    uint2 dimensions;
    uint mipLevel = 0, numberOfLevels;
    positionTexture.GetDimensions(mipLevel, dimensions.x, dimensions.y, numberOfLevels);
    
    float4 position = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], pin.texcoord); // worldSpace
    float3 normal = normalTexture.Sample(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord).xyz; // worldSpace

    float4 positionFrom = position;
    float4 positionTo = positionFrom;
    
#if 0
    float3 incident = normalize(position);
#else
    float3 incident = normalize(position.xyz - cameraPositon.xyz);
#endif
    float3 reflection = normalize(reflect(incident, normal.xyz));

    float4 startWorld = float4(positionFrom.xyz + (reflection * 0), 1);
    float4 endWorld = float4(positionFrom.xyz + (reflection * maxDistance), 1);
    if (endWorld.z < 0)
    {
        float3 v = endWorld.xyz - startWorld.xyz;
        endWorld.xyz = startWorld.xyz + v * abs(startWorld.z / v.z);
    }

    //float4 startFrag = mul(startWorld, projection); // from view to clipSpace
    float4 startFrag = mul(startWorld, viewProjection); // from world to clipSpace
    startFrag /= startFrag.w; //from clipSpave to ndc
    startFrag.xy = NdcToUv(startFrag.xy); // from uv to fragment/pixel coordinate
    startFrag.xy *= dimensions;
    
    //float4 endFrag = mul(endWorld, projection); //from world to clipSpace
    float4 endFrag = mul(endWorld, viewProjection); //from world to clipSpace
    endFrag /= endFrag.w; //from clipSpace to ndc
    endFrag.xy = NdcToUv(endFrag.xy); //from ndc to uv
    endFrag.xy *= dimensions;
    
    float2 frag = startFrag.xy;
    
    float4 uv = 0;
    uv.xy = frag / dimensions;
    
    float deltaX = endFrag.x - startFrag.x;
    float deltaY = endFrag.y - startFrag.y;
    
    float useX = abs(deltaX) >= abs(deltaY) ? 1 : 0;
    float delta = lerp(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);
    
    float2 increment = float2(deltaX, deltaY) / max(delta, 0.001);
    
    float search0 = 0;
    float search1 = 0;
    
    int hit0 = 0;
    int hit1 = 0;
    
    float viewDistance = startWorld.z;
    float depth = thickness;
    
#define MAX_DELTA 64
    delta = min(MAX_DELTA, delta);
    [unroll(MAX_DELTA)]
    for (int i = 0; i < (int) delta; ++i)
    {
        frag += increment;
        uv.xy = frag / dimensions;
        if (uv.x <= 0 || uv.x >= 1 || uv.y <= 0 || uv.y >= 1)
        {
            hit0 = 0;
            break;
        }
#if 0
        positionTo = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy); //viewSpace
#else
        positionTo = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy); // worldSpace
        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), viewProjection);
        positionToClip /= positionToClip.w;
#endif   
        search1 = lerp((frag.y - startFrag.y) / deltaY, (frag.x - startFrag.x) / deltaX, useX);
        search1 = clamp(search1, 0.0, 1.0);
        
        // Perspective Correct Interpolation
        // NDC.z ƒx[ƒX‚Å”äŠr‚·‚é
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
#if 0
        if (depth > 0 && depth < thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#else
        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit0 = 1;
            break;
        }
        else
        {
            search0 = search1;
        }
#endif
    }
#if 0
    hit1 = hit0;
#else
    search1 = search0 + ((search1 - search0) / 2.0);
    steps *= hit0;
    
    [unroll]
    for (i = 0; i < steps; ++i)
    {
        frag = lerp(startFrag.xy, endFrag.xy, search1);
        uv.xy = frag / dimensions;
        
        positionTo = positionTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy); //viewSpace
        float4 positionToClip = mul(float4(positionTo.xyz, 1.0), viewProjection);
        positionToClip /= positionToClip.w;
        
        // PerspectiveCorrect Interpolation
#if 0
        viewDistance = (startWorld.z * endWorld.z) / lerp(endWorld.z, startWorld.z, search1);
        depth = viewDistance - positionTo.z;
        
        if (depth > 0 && depth < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#else
        float interpolatedZ = lerp(startFrag.z, endFrag.z, search1);
        float depthDiff = interpolatedZ - positionToClip.z;

        if (depthDiff > 0 && depthDiff < thickness)
        {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2);
        }
        else
        {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2);
            search0 = temp;
        }
#endif
    }
#endif
    float visibility = hit1;
    visibility *= (1 - max(dot(-normalize(positionFrom.xyz), reflection), 0));
    visibility *= (1 - clamp(depth / thickness, 0, 1));
    visibility *= positionTo.w;
    visibility *= (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) ? 0 : 1;
    visibility = clamp(visibility, 0, 1);
    
    float fresnel = saturate(FSchlick(0.04, max(0, dot(reflection, normal.xyz))));
    float3 reflectionColor = colorTexture.Sample(samplerStates[LINEAR_BORDER_WHITE], uv.xy).rgb;
    reflectionColor = fresnel * reflectionColor * visibility * reflectionIntensity;
    return reflectionColor;
}