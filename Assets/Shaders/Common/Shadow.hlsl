//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 11:49:33
//

static const float3 PCF_POISSON_DISK[20] = {
   float3( 1,  1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1,  1,  1), 
   float3( 1,  1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1,  1, -1),
   float3( 1,  1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1,  1,  0),
   float3( 1,  0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1,  0, -1),
   float3( 0,  1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0,  1, -1)
};

float PCFCascade(
    Texture2D<float> ShadowMap,
    SamplerComparisonState comparisonSampler,
    float4 WorldSpacePosition,
    float3 LightDirection,
    float4x4 LightView,
    float4x4 LightProj,
    float bias,
    int kernelSize)
{
    // Transform world-space position into light space
    float4 lightSpacePosition = mul(LightView, WorldSpacePosition);
    float4 ndcPosition = mul(LightProj, lightSpacePosition);
    ndcPosition.xyz /= ndcPosition.w;

    // Compute shadow map UV coordinates
    float2 shadowUV = ndcPosition.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    // Check if outside the light frustum
    if (ndcPosition.z > 1.0)
        return 1.0;

    // Compute texel size for PCF kernel sampling
    uint shadowWidth, shadowHeight;
    ShadowMap.GetDimensions(shadowWidth, shadowHeight);
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);

    // Perform PCF with the comparison sampler
    float shadow = 0.0;
    int sampleCount = 0;

    for (int x = -kernelSize; x <= kernelSize; x++) {
        for (int y = -kernelSize; y <= kernelSize; y++) {
            // Offset UV coordinates for sampling
            float2 offsetUV = shadowUV + float2(x, y) * texelSize;

            // Use the comparison sampler to perform the depth test
            shadow += ShadowMap.SampleCmpLevelZero(comparisonSampler, offsetUV, ndcPosition.z - bias);
            sampleCount++;
        }
    }
    shadow /= sampleCount;
    
    return shadow;
}

float PCFPoint(
    TextureCube<float> ShadowMap,
    SamplerState comparisonSampler,
    float4 WorldSpacePosition,
    float3 CameraPosition,
    float3 LightPosition,
    int kernelSize)
{
    float3 fragToLight = WorldSpacePosition.xyz - LightPosition;
    fragToLight.y = -fragToLight.y;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.05;

    // Define an offset array for 2x2 sampling (cheap PCF)
    float3 offset[4] = {
        float3( 0.5,  0.5, 0.0),
        float3(-0.5,  0.5, 0.0),
        float3( 0.5, -0.5, 0.0),
        float3(-0.5, -0.5, 0.0)
    };

    // Sample multiple points within the kernel
    for (int i = 0; i < 4; ++i)
    {
        float3 sampleDirection = fragToLight + offset[i] * 0.01; // Small jitter for sampling
        float closestDepth = ShadowMap.Sample(comparisonSampler, sampleDirection).r;
        closestDepth *= 25;

        // Accumulate shadow contribution
        if (currentDepth - bias <= closestDepth) {
            shadow += 0.25; // Weight of each sample (1/4 for 2x2 PCF)
        }
    }

    return shadow;
}

float PCFSpot(
    Texture2D<float> ShadowMap,
    SamplerComparisonState comparisonSampler,
    float4 WorldSpacePosition,
    column_major float4x4 LightView,
    column_major float4x4 LightProj)
{
    float4 lightSpacePos = mul(LightProj, mul(LightView, WorldSpacePosition));
    float3 lightSpaceNDC = lightSpacePos.xyz / lightSpacePos.w;

    float2 shadowMapUV = lightSpaceNDC.xy * 0.5 + 0.5;
    shadowMapUV.y = 1.0 - shadowMapUV.y;

    float shadow = 0.0f;
    float depth = lightSpaceNDC.z;

    uint shadowMapWidth, shadowMapHeight;
    ShadowMap.GetDimensions(shadowMapWidth, shadowMapHeight);

    float2 texelSize = float2(1.0 / shadowMapWidth, 1.0 / shadowMapHeight);
    float2 pcfKernel[4] = {
        float2(-1.0, -1.0) * texelSize, 
        float2(1.0, -1.0) * texelSize,
        float2(-1.0, 1.0) * texelSize, 
        float2(1.0, 1.0) * texelSize
    };

    int sampleCount = 0;
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            // Offset UV coordinates for sampling
            float2 offsetUV = shadowMapUV + float2(x, y) * texelSize;

            // Use the comparison sampler to perform the depth test
            shadow += ShadowMap.SampleCmpLevelZero(comparisonSampler, offsetUV, depth - 0.005);
            sampleCount++;
        }
    }
    shadow /= sampleCount;

    return shadow; 
}
