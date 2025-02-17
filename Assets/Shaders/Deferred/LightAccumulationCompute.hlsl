//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-07 14:42:28
//

#include "Assets/Shaders/Common/ShaderUtils.hlsl"
#include "Assets/Shaders/Common/Math.hlsl"

struct PushConstants
{
    uint DepthTexture;
    uint AlbedoTexture;
    uint NormalTexture;
    uint OutputTexture;

    int PBR;
    int Irradiance;
    int Prefilter;
    int BRDF;

    float3 CameraPosition;
    int CubeSampler;

    int RegularSampler;
    int3 Pad;

    column_major float4x4 InverseViewProj;
};

ConstantBuffer<PushConstants> Settings : register(b0);

float4 GetPositionFromDepth(float2 uv, float depth)
{
    // Don't need to normalize -- directx depth is [0; 1]
    float z = depth;

    float4 clipSpacePosition = float4(uv * 2.0 - 1.0, z, 1.0);
    clipSpacePosition.y *= -1.0f;

    float4 viewSpacePosition = mul(Settings.InverseViewProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    RWTexture2D<float4> output = ResourceDescriptorHeap[Settings.OutputTexture];

    int width, height;
    output.GetDimensions(width, height);
    if (ThreadID.x > width || ThreadID.y > height) return;

    //
    Texture2D<float> depth = ResourceDescriptorHeap[Settings.DepthTexture];
    Texture2D<float4> albedo = ResourceDescriptorHeap[Settings.AlbedoTexture];
    Texture2D<float4> normal = ResourceDescriptorHeap[Settings.NormalTexture];
    Texture2D<float2> pbr = ResourceDescriptorHeap[Settings.PBR];
    TextureCube<half4> Irradiance = ResourceDescriptorHeap[Settings.Irradiance];
    TextureCube<half4> Prefilter = ResourceDescriptorHeap[Settings.Prefilter];
    Texture2D<float2> BRDF = ResourceDescriptorHeap[Settings.BRDF];
    SamplerState RegularSampler = SamplerDescriptorHeap[Settings.RegularSampler];
    SamplerState CubeSampler = SamplerDescriptorHeap[Settings.CubeSampler];

    float2 uv = TexelToUV(ThreadID.xy, 1.0 / float2(width, height));
    float ndcDepth = depth.Load(int3(ThreadID.xy, 0));
    float4 position = GetPositionFromDepth(uv, ndcDepth);
    float4 color = albedo.Load(ThreadID);

    float metallic = pbr.Load(ThreadID).r;
    float roughness = pbr.Load(ThreadID).g;
    
    float3 N = normalize(normal.Load(ThreadID).xyz);
    float3 V = normalize(Settings.CameraPosition - position.xyz);
    float3 R = reflect(-V, N);

    float NdotV = max(0.5, dot(N, V));
    float3 Lr = 2.0 * NdotV * N - V;
    float3 Lo = 0.0;

    float3 F0 = 0.04;
    F0 = lerp(F0, color.xyz, metallic);

    float3 directLighting = 0.0;
    {}

    float3 indirectLighting = 0.0;
    {
        float3 irradiance = Irradiance.Sample(CubeSampler, N).rgb;
        float3 F = FresnelSchlick(NdotV, F0);
        float3 kd = lerp(1.0 - F, 0.0, metallic);
        float3 diffuseIBL = kd * color.rgb;

        uint maxReflectionLOD = 5;
        float lod = roughness * maxReflectionLOD;
        lod = clamp(lod, 0, maxReflectionLOD); // Ensure it does not go out of bounds
        float3 specularIrradiance = Prefilter.SampleLevel(CubeSampler, Lr, lod).rgb;

        float2 brdfUV = float2(NdotV, roughness);
        float2 specularBRDF = BRDF.Sample(RegularSampler, brdfUV).rg;
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
        indirectLighting = (diffuseIBL + specularIBL) * irradiance;
    }

    float3 final = directLighting + indirectLighting;
    output[ThreadID.xy] = float4(final, 1.0);
}
