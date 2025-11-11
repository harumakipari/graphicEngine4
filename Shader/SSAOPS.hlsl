#include "Sampler.hlsli"
#include "FullScreenQuad.hlsli"
#include "Constants.hlsli"

Texture2D depthTexture : register(t0);

float2 ndc_to_uv(float2 ndc)
{
    float2 uv;
    uv.x = 0.5 + 0.5 * ndc.x;
    uv.y = 0.5 - ndc.y * 0.5;
    return uv;
}

cbuffer SSAO_CONSTANTS_BUFFER : register(b5)
{
    float sigma;
    float power;
    bool improvedNormalReconstructionFromDepth;
    bool bilateralBlur;
}

static const float3 kernel_points[64] =
{
    { 0.00868211500, -0.0427273996, 0.0337389559 },
    { 0.0422878861, 0.0255667437, 0.0421174169 },
    { 0.00133369095, -0.000206216064, 0.000314872246 },
    { 0.00550761260, 0.0445004962, 0.0369733199 },
    { 0.00802796893, 0.000712828245, 0.0389925502 },
    { -0.00727070076, 0.00919260643, 0.00159413333 },
    { 0.0814671144, 0.0169006232, 0.108687885 },
    { 0.0345553271, -0.0601100996, 0.0623369254 },
    { 0.0575188287, -0.117980719, 0.0903726891 },
    { 0.0338569917, -0.0841888934, 0.0627225488 },
    { 0.0592471771, -0.169112802, 0.0244632196 },
    { -0.0238562804, 0.105675779, 0.209886223 },
    { 0.0631165877, 0.0301473364, 0.0615524314 },
    { -0.0134508293, -0.0277324617, 0.00691395300 },
    { 0.114152357, 0.102236181, 0.111014776 },
    { -0.0221489109, -0.00430791033, 0.0260636527 },
    { -0.0271239709, 0.0771735162, 0.0862243697 },
    { 0.298623502, -0.137056544, 0.0121193761 },
    { -0.217831656, -0.111817703, 0.126772851 },
    { 0.0746574551, -0.00350309932, 0.114315465 },
    { -0.0230329111, 0.106690325, 0.0136655448 },
    { -0.0474987105, 0.0584983639, 0.0424890034 },
    { 0.0188070145, 0.0568048507, 0.0293925218 },
    { -0.0112051172, 0.179476753, 0.122986622 },
    { 0.0295139719, 0.162125558, 0.0753630325 },
    { -0.00302972272, -0.0406616218, 0.000857106352 },
    { 0.00778160710, 0.0270030703, 0.0350351147 },
    { 0.0268135704, 0.183430821, 0.0638199970 },
    { 0.130813897, 0.00156025402, 0.224606648 },
    { -0.147132352, 0.152517274, 0.183435142 },
    { -0.0907125175, -0.00177383155, 0.160256311 },
    { -0.0266149752, -0.330671698, 0.0708868578 },
    { 0.0116967773, -0.117801137, 0.0648231357 },
    { -0.278127432, 0.243157208, 0.248168632 },
    { -0.0433821045, 0.0629939660, 0.0638747588 },
    { -0.341667235, 0.0813603401, 0.283274621 },
    { -0.294411331, 0.218125328, 0.119183138 },
    { 0.124180131, -0.377155364, 0.268380553 },
    { 0.0290975738, 0.110363945, 0.0442696661 },
    { -0.345448762, -0.294173360, 0.357681096 },
    { -0.198425651, 0.117884018, 0.280610353 },
    { -0.0471894965, 0.167634696, 0.0171351191 },
    { 0.283999801, 0.307914823, 0.0745619610 },
    { 0.278367370, 0.143483743, 0.0777348951 },
    { -0.00319555169, 0.0113931699, 0.0792912468 },
    { 0.408831716, -0.255240113, 0.389685363 },
    { 0.0870304331, 0.131976292, 0.0758789256 },
    { 0.508056879, -0.226139233, 0.0249240790 },
    { 0.443417460, -0.0719382539, 0.0811995938 },
    { -0.405097991, -0.0126497196, 0.468290806 },
    { -0.271491766, 0.266288817, 0.173985392 },
    { -0.454723060, 0.203438774, 0.416500956 },
    { -0.225664973, 0.251260072, 0.130069673 },
    { 0.0662298575, 0.152405098, 0.0794597119 },
    { 0.214381784, -0.0221292414, 0.277178794 },
    { -0.543329179, -0.327755034, 0.367753297 },
    { -0.107578896, -0.311785251, 0.116201803 },
    { -0.138324469, -0.586842418, 0.159054667 },
    { -0.535777450, -0.474822164, 0.298666269 },
    { 0.525617778, -0.189941183, 0.353068262 },
    { 0.418803364, 0.0256034732, 0.492332071 },
    { 0.237295657, 0.280489057, 0.375523627 },
    { 0.146710590, -0.156013578, 0.0835350007 },
    { -0.173016176, -0.680333912, 0.495088488 },
};

static const float3 noise[16] =
{
    { 0.772417068, -0.423055708, 0.00000000 },
    { -0.714291692, 0.509235382, 0.00000000 },
    { -0.780199528, -0.183640003, 0.00000000 },
    { 0.472295880, 0.162610054, 0.00000000 },
    { -0.442540765, -0.263137937, 0.00000000 },
    { -0.960392714, -0.472289205, 0.00000000 },
    { -0.347602427, 0.830676079, 0.00000000 },
    { -0.573735476, -0.472531378, 0.00000000 },
    { -0.629811406, -0.642059684, 0.00000000 },
    { -0.352298439, -0.320050240, 0.00000000 },
    { 0.599728346, 0.426071405, 0.00000000 },
    { -0.173549354, 0.653944254, 0.00000000 },
    { -0.287484705, -0.0727353096, 0.00000000 },
    { 0.324079156, 0.534158945, 0.00000000 },
    { 0.968484163, -0.229418755, 0.00000000 },
    { -0.377048135, 0.338529825, 0.00000000 },
};

float3 reconstruct_position(in float2 uv, in float z, in float4x4 inverse_projection)
{
    float x = uv.x * 2.0 - 1.0;
    float y = (1.0 - uv.y) * 2.0 - 1.0;
    float4 position_s = float4(x, y, z, 1.0);
    float4 position_v = mul(position_s, inverse_projection);
    return position_v.xyz / position_v.w;
}

float main(VS_OUT pin) : SV_TARGET
{
    float3 position = 0;
    float3 normal = 0;
    if (improvedNormalReconstructionFromDepth)
    {
		//Improved normal reconstruction from depth
        uint mip_level = 0, number_of_samples;
        uint2 depth_dimensions;
        depthTexture.GetDimensions(mip_level, depth_dimensions.x, depth_dimensions.y, number_of_samples);
	
        float2 uv[5];
        uv[0] = pin.texcoord; // center
        uv[1] = pin.texcoord + float2(1, 0) / depth_dimensions; // right 
        uv[2] = pin.texcoord + float2(-1, 0) / depth_dimensions; // left
        uv[3] = pin.texcoord + float2(0, 1) / depth_dimensions; // down
        uv[4] = pin.texcoord + float2(0, -1) / depth_dimensions; // up

        float depth[5];
        depth[0] = depthTexture.SampleLevel(samplerStates[LINEAR_CLAMP], uv[0], 0).r; // center
        depth[1] = depthTexture.SampleLevel(samplerStates[LINEAR_CLAMP], uv[1], 0).r; // right 
        depth[2] = depthTexture.SampleLevel(samplerStates[LINEAR_CLAMP], uv[2], 0).r; // left
        depth[3] = depthTexture.SampleLevel(samplerStates[LINEAR_CLAMP], uv[3], 0).r; // down
        depth[4] = depthTexture.SampleLevel(samplerStates[LINEAR_CLAMP], uv[4], 0).r; // up

        float3 p[5];
        p[0] = reconstruct_position(uv[0], depth[0], inverseProjection); // center
        p[1] = reconstruct_position(uv[1], depth[1], inverseProjection); // right 
        p[2] = reconstruct_position(uv[2], depth[2], inverseProjection); // left
        p[3] = reconstruct_position(uv[3], depth[3], inverseProjection); // down
        p[4] = reconstruct_position(uv[4], depth[4], inverseProjection); // up
	
        const uint best_z_horizontal = abs(p[1].z - p[0].z) < abs(p[2].z - p[0].z) ? 1 : 2;
        const uint best_z_vertical = abs(p[3].z - p[0].z) < abs(p[4].z - p[0].z) ? 3 : 4;
#if 0
		float3 p1 = 0, p2 = 0;
		if (best_z_horizontal == 1 /*right*/ && best_z_vertical == 4 /*right*/)
		{
			p1 = p[1]; // right 
			p2 = p[4]; // up
		}
		else if (best_z_horizontal == 1 /*right*/ && best_z_vertical == 3 /*down*/)
		{
			p1 = p[3]; // down
			p2 = p[1]; // right 
		}
		else if (best_z_horizontal == 2 /*left*/ && best_z_vertical == 4 /*up*/)
		{
			p1 = p[4]; // up
			p2 = p[2]; // left
		}
		else if (best_z_horizontal == 2 /*left*/ && best_z_vertical == 3 /*down*/)
		{
			p1 = p[2]; // left
			p2 = p[3]; // down
		}

		normal = normalize(cross(p2 - p[0], p1 - p[0]));
#else
        normal = normalize(cross(p[best_z_horizontal] - p[0], p[best_z_vertical] - p[0]));
        normal = dot(normal, p[0]) > 0 ? -normal : normal;
#endif
        position = p[0];
    }
    else
    {
        float depth = depthTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], pin.texcoord, 0).r;
        position = reconstruct_position(pin.texcoord, depth, inverseProjection); // view space
        normal = normalize(cross(ddx(position), ddy(position)));
    }

	// TBN is a matrix to transform from tangent to view-space matrix
    float3 random_vec = noise[(pin.position.x % 4) + 4 * (pin.position.y % 4)]; // Random kernel rotation
    float3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
	
    const int kernel_size = 64;
	
    float occlusion = 0.0; // accumulated value
    for (int kernel = 0; kernel < kernel_size; ++kernel)
    {
        float3 sample_position = mul(kernel_points[kernel], TBN); // from tangent to view-space
        const float radius = 1.0;
        sample_position = position.xyz + sample_position * radius;
		
		// Find a view-space scene intersection point on the ray.
        float4 intersection = mul(float4(sample_position, 1.0), projection); // from view to clip-space
        intersection /= intersection.w; // from clip-space to ndc
        intersection.z = depthTexture.SampleLevel(samplerStates[LINEAR_BORDER_BLACK], ndc_to_uv(intersection.xy), 0).x;
        intersection = mul(intersection, inverseProjection); // from ndc to view-space
        intersection /= intersection.w; // perspective divide
		
		// Alchemy AO
        float3 v = intersection.xyz - position.xyz;
        const float beta = 0.0; // bias distance
        const float epsilon = 0.001;
        occlusion += max(0, dot(normal, v) - position.z * beta) / (dot(v, v) + epsilon);
    }
    occlusion = max(0.0, 1.0 - (2.0 * sigma * occlusion / kernel_size));
    occlusion = pow(occlusion, power);
	
    return occlusion;
}
