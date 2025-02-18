//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-07 14:42:28
//

#include "Assets/Shaders/Common/ShaderUtils.hlsl"
#include "Assets/Shaders/Common/PBR.hlsl"
#include "Assets/Shaders/Common/Light.hlsl"

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
    int LightBuffer;
    int2 Pad;

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

float3 CalcPointLight(float3 world, PointLight light, float3 V, float3 N, float3 F0, float roughness, float metallic, float3 albedo)
{
    float3 lightPos = light.Position;
    float3 lightColor = light.Color;

    float distance = length(lightPos - world) + Epsilon;
    float attenuation = 1.0 / (distance * distance);
    if (attenuation <= 0.0)
        return 0.0;

    float3 L = normalize(lightPos - world);
    float3 H = normalize(V + L);
    float3 radiance = lightColor * attenuation;
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), Epsilon), F0);
    float3 numerator = F * G * NDF;
    float denominator = 4 * max(dot(N, V), Epsilon) * max(dot(N, L), Epsilon) + Epsilon;
    float3 specular = numerator / denominator;
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
    float NdotL = max(dot(N, L), Epsilon);
    return (kD * albedo / PI + specular) * radiance * NdotL * (light.Radius * light.Radius);
}

float3 CalcDirectionalLight(DirectionalLight light, float3 V, float3 N, float3 F0, float roughness, float metallic, float3 albedo)
{
    float3 L = normalize(-light.Direction);
    float attenuation = clamp(dot(N, -L), 0.0, 1.0);
    if (attenuation > 0.0f)
        return 0.0;

    float3 lightColor = light.Color;   
    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), Epsilon);
    float3 radiance = lightColor * NdotL;
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), Epsilon), F0);
    float3 numerator = F * G * NDF;
    float denominator = 4 * max(dot(N, V), Epsilon) * max(dot(N, L), Epsilon) + Epsilon;
    float3 specular = numerator / denominator;
    float3 kS = F;
    float3 kD = 1.0 - F;
    kD *= 1.0 - metallic;
    return (kD * albedo.xyz / PI + specular) * radiance * light.Strength;
}

float3 CalcSpotLight(float3 world, SpotLight light, float3 V, float3 N, float3 F0, float roughness, float metallic, float3 albedo)
{   
    float3 L = normalize(light.Position - world);  // Light direction towards the fragment
    float3 H = normalize(V + L);                   // Halfway vector

    float distance = length(light.Position - world);
    float theta = dot(L, normalize(-light.Direction)); // Angle between light direction and surface
    float smoothFactor = smoothstep(cos(radians(light.OuterRadius)), cos(radians(light.Radius)), theta);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return 0.0; // No lighting contribution if the surface faces away

    // Distance attenuation (optional)
    float attenuation = 1.0 / (distance * distance);
    
    // Lambertian diffuse lighting
    float3 radiance = light.Color * NdotL * light.Strength * smoothFactor * attenuation;

    // Cook-Torrance BRDF calculations
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), Epsilon), F0);

    float3 numerator = F * G * NDF;
    float denominator = 4.0 * max(dot(N, V), Epsilon) * max(dot(N, L), Epsilon) + Epsilon;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    return (kD * albedo / PI + specular) * radiance;
}

uint GetMaxReflectionLOD()
{
    TextureCube Prefilter = ResourceDescriptorHeap[Settings.Prefilter];

    int w, h, l;
    Prefilter.GetDimensions(0, w, h, l);
    return l;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    // Initial setup
    RWTexture2D<float4> output = ResourceDescriptorHeap[Settings.OutputTexture];
    int width, height;
    output.GetDimensions(width, height);
    if (ThreadID.x > width || ThreadID.y > height) return;

    // Load textures and render targets
    Texture2D<float> depth = ResourceDescriptorHeap[Settings.DepthTexture];
    Texture2D<float4> albedo = ResourceDescriptorHeap[Settings.AlbedoTexture];
    Texture2D<float4> normal = ResourceDescriptorHeap[Settings.NormalTexture];
    Texture2D<float2> pbr = ResourceDescriptorHeap[Settings.PBR];
    TextureCube<half4> Irradiance = ResourceDescriptorHeap[Settings.Irradiance];
    TextureCube<half4> Prefilter = ResourceDescriptorHeap[Settings.Prefilter];
    Texture2D<float2> BRDF = ResourceDescriptorHeap[Settings.BRDF];

    // Load samplers
    SamplerState RegularSampler = SamplerDescriptorHeap[Settings.RegularSampler];
    SamplerState CubeSampler = SamplerDescriptorHeap[Settings.CubeSampler];

    // Load buffers
    ConstantBuffer<LightData> lightData = ResourceDescriptorHeap[Settings.LightBuffer];
    StructuredBuffer<DirectionalLight> directionalLights = ResourceDescriptorHeap[lightData.DirLightSRV];
    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[lightData.PointLightSRV];
    StructuredBuffer<SpotLight> spotLights = ResourceDescriptorHeap[lightData.SpotLightSRV];

    // Initial variables
    float2 uv = TexelToUV(ThreadID.xy, 1.0 / float2(width, height));
    float ndcDepth = depth.Load(int3(ThreadID.xy, 0));
    float4 position = GetPositionFromDepth(uv, ndcDepth);
    float4 color = albedo.Load(ThreadID);
    float metallic = pbr.Load(ThreadID).r;
    float roughness = pbr.Load(ThreadID).g;
    
    // Initial lighting variables
    float3 N = normalize(normal.Load(ThreadID).xyz);
    float3 V = normalize(Settings.CameraPosition - position.xyz);
    float3 R = reflect(-V, N);

    float NdotV = max(0.5, dot(N, V));
    float3 Lr = 2.0 * NdotV * N - V;
    float3 Lo = 0.0;
    float3 F0 = lerp(0.04, color.xyz, metallic);

    // Direct lighting calculation
    float3 directLighting = 0.0;
    {
        for (int i = 0; i < lightData.PointLightCount; i++) {
            directLighting += CalcPointLight(position.xyz, pointLights[i], V, N, F0, roughness, metallic, color.rgb);
        }
        for (int i = 0; i < lightData.DirLightCount; i++) {
            directLighting += CalcDirectionalLight(directionalLights[i], V, N, F0, roughness, metallic, color.rgb);
        }
        for (int i = 0; i < lightData.SpotLightCount; i++) {
            directLighting += CalcSpotLight(position.xyz, spotLights[i], V, N, F0, roughness, metallic, color.rgb);
        }
    }

    // Indirect lighting calculation
    float3 indirectLighting = 0.0;
    {
        float3 irradiance = Irradiance.Sample(CubeSampler, N).rgb;
        float3 F = FresnelSchlick(NdotV, F0);
        float3 kd = lerp(1.0 - F, 0.0, metallic);
        float3 diffuseIBL = kd * color.rgb;

        uint maxReflectionLOD = GetMaxReflectionLOD();
        float3 specularIrradiance = Prefilter.SampleLevel(CubeSampler, Lr, roughness * maxReflectionLOD).rgb;
        
        float2 brdfUV = float2(NdotV, roughness);
        float2 specularBRDF = BRDF.Sample(RegularSampler, brdfUV).rg;
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
        indirectLighting = (diffuseIBL + specularIBL) * irradiance;
    }

    //
    float3 final = directLighting + (indirectLighting * 1.0);
    output[ThreadID.xy] = float4(final, 1.0);
}
