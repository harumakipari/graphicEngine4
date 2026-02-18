#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"
#include "Lights.hlsli"
#include "Sampler.hlsli"

Texture2D depth_map : register(t0);
Texture2DArray shadow_map : register(t1);

Texture2D noise2d_map : register(t30);
Texture3D noise3d_map : register(t31);

// Z buffer to linear 0..1 depth
inline float linear_01_depth(float z, float near, float far)
{
    return 1.0 / ((1 - far / near) * z + (far / near));
}
float mie_scattering(float cos_angle, float g)
{
    return (1.0 / (4.0 * 3.14159265358979)) * ((1 - (g * g)) / (pow((1 + (g * g)) - (2 * g) * cos_angle, 1.5)));
}
void apply_height_fog(float3 position /*world_space*/, inout float density)
{
    const float ground_level = groundLevel;
    const float height_scale = fogHeightFalloff;
    density *= exp(-(position.y - ground_level) * height_scale);
}
float get_density(float3 position /*world_space*/)
{
    float density = 1;
	
#if 1
    const float time = elapsedTime * timeScale;

    const float noise_scale = noiseScale;
    const float3 noise_velocity = normalize(float3(1, 0, 0));
    float noise = 0.5 * noise3d_map.Sample(samplerStates[LINEAR], position * noise_scale + noise_velocity * time).x + 0.5;
	
    const float noise_intensity_offset = 0.2;
    const float noise_intensity = fogDensity;
	
    noise = saturate(noise - noise_intensity_offset) * noise_intensity;
    density = saturate(noise);
#endif
	
    apply_height_fog(position, density);

    return density;
}
float get_light_attenuation(float3 position_world_space)
{
    float depth = length(position_world_space - cameraPositon.xyz);
	
    int cascade_index = -1;
    for (uint layer = 0; layer < 4; ++layer)
    {
        float distance = ((float[4]) (cascadedPlaneDistances[layer / 4]))[layer % 4];
        if (distance > depth)
        {
            cascade_index = layer;
            break;
        }
    }
    if (cascade_index == -1)
    {
        return 1;
    }
    float4 position_light_space = mul(float4(position_world_space, 1.0), cascadedMatrices[cascade_index]);
    position_light_space /= position_light_space.w;

	// To texture space
    position_light_space.x = position_light_space.x * +0.5 + 0.5;
    position_light_space.y = position_light_space.y * -0.5 + 0.5;
	
    float atten = shadow_map.SampleCmpLevelZero(comparisionSamplerState, float3(position_light_space.xy, cascade_index), position_light_space.z - shadowDepthBias).x;
	
#if 0
	const float shadow_strength = 0.2;
	atten = shadow_strength + atten * (1 - shadow_strength);
#endif	
    return atten;
}
float4 dithered_ray_march(float2 screen_pos, float3 ray_start, float3 ray_dir, float ray_length)
{
#if 1
    const float4x4 dither_pattern =
    {
        { 0.0f, 0.5f, 0.125f, 0.625f },
        { 0.75f, 0.22f, 0.875f, 0.375f },
        { 0.1875f, 0.6875f, 0.0625f, 0.5625 },
        { 0.9375f, 0.4375f, 0.8125f, 0.3125 }
    };
    float dither_value = dither_pattern[screen_pos.x % 4][screen_pos.y % 4];
#else
	float dither_value = 0;
#endif
	
    const int step_count = 16;

    float step_size = ray_length / step_count;
    float3 step = ray_dir * step_size;
	
    float3 current_position = ray_start + step * dither_value;

    float4 vlight = 0;

    float extinction = 0;
    float cos_angle = dot(normalize(lightDirection.xyz), -ray_dir);
	
	[loop]
    for (int i = 0; i < step_count; ++i)
    {
        float atten = get_light_attenuation(current_position);
		
		
		
		
		
		//float density = get_density(current_position);
        float density = 1;
#if 1
        const float time = elapsedTime * timeScale;

        const float noise_scale = noiseScale;
        const float3 noise_velocity = normalize(float3(1, 0, 0));
		
        float3 position = frac(current_position * noise_scale + noise_velocity * time);
        float noise = 0.5 * noise3d_map.Sample(samplerStates[LINEAR], position).x + 0.5;
        const float sharpness_factor = 1.0;
        noise = pow(noise, sharpness_factor);
        const float noise_intensity_offset = 0.0;
        const float noise_intensity = fogDensity;
        density = max(0, noise - noise_intensity_offset) * noise_intensity;
#endif
        apply_height_fog(current_position, density);

		
		
		
		
		
        const float scattering_coef = 0.815f;
        const float extinction_coef = 0.0031f;
        float scattering = scattering_coef * step_size * density;
        extinction += extinction_coef * step_size * density;

        float4 light = atten * scattering * exp(-extinction);
        vlight += light;

        current_position += step;
    }

#if 1
	// apply phase function for dir light
	//const float mie_g = 0.75;
    const float mie_g = mieScatteringCoef;
    vlight *= mie_scattering(cos_angle, mie_g);
#endif

	// apply light's color
    vlight.rgb *= fogColor.rgb * fogColor.a;

    vlight = max(0, vlight);
#if 1 
    vlight.w = exp(-extinction);
#endif
	
    return saturate(vlight);
}

float4 main(VS_OUT pin) : SV_TARGET
{
#if 0
	float depth = depth_map.Sample(sampler_states[POINT_WRAP], pin.texcoord).x;
#else
	// downsample depth
    float4 sampled_depth = depth_map.Gather(samplerStates[POINT], pin.texcoord);
    float min_depth = min(min(sampled_depth.x, sampled_depth.y), min(sampled_depth.z, sampled_depth.w));
    float max_depth = max(max(sampled_depth.x, sampled_depth.y), max(sampled_depth.z, sampled_depth.w));
	// chessboard pattern
    int2 position = pin.position.xy % 2;
    int index = position.x + position.y;
    float depth = index == 1 ? min_depth : max_depth;
#endif
	
    float4 position_ndc;
	// texture space to ndc
    position_ndc.x = pin.texcoord.x * +2 - 1;
    position_ndc.y = pin.texcoord.y * -2 + 1;
    position_ndc.z = depth;
    position_ndc.w = 1;

	// ndc to world space
    float4 position_world_space = mul(position_ndc,inverseViewProjection);
    position_world_space = position_world_space / position_world_space.w;
	
    float3 ray_start = cameraPositon.xyz;
    float3 ray_dir = position_world_space.xyz - cameraPositon.xyz;
	
#if 0
	// extract near and far values from perspective projection matrix
	float near = -scene_data.projection._43 / scene_data.projection._33;
	float far = -scene_data.projection._33 / (1 - scene_data.projection._33) * near;
	float linear_depth = linear_01_depth(depth, near, far);
	ray_dir *= linear_depth;
#endif
    float ray_length = length(ray_dir);
    ray_dir /= ray_length;
	
#if 1
    const float max_ray_length = 1000;
    //const float max_ray_length = effect_data.max_ray_length;
    ray_length = min(ray_length, max_ray_length);
#endif
	
    float4 color = dithered_ray_march(pin.position.xy, ray_start, ray_dir, ray_length);
	
#if 0
	if (linear_depth > 0.999999)
	{
		const float skybox_extinction_coef = 0.9;
		color.w = lerp(color.w, 1, 1.0f - skybox_extinction_coef);
	}
#endif
	
	//color.a = depth;
	
    return color;
}
