//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 15:16:30
//

struct Input
{
    int InputID;
    int OutputID;
    int SamplerID;
    int Pad;  
};

ConstantBuffer<Input> Inputs : register(b0);

uint2 ClampPixel(float2 textureSize, uint2 pos)
{
    return uint2(clamp(pos.x, 0u, textureSize.x - 1), clamp(pos.y, 0u, textureSize.y - 1));
}

float4 ApplyFXAA(Texture2D<float4> texture, SamplerState samplerState, uint2 currPixelPosition, float2 texelSize, uint2 textureSize)
{
    // Sample colors at the current pixel and its neighbors (with clamping)
    float4 colorCenter = texture.SampleLevel(samplerState, currPixelPosition * texelSize, 0);
    float4 colorUp     = texture.SampleLevel(samplerState, ClampPixel(currPixelPosition + uint2(0, -1), textureSize) * texelSize, 0);
    float4 colorDown   = texture.SampleLevel(samplerState, ClampPixel(currPixelPosition + uint2(0, 1), textureSize) * texelSize, 0);
    float4 colorLeft   = texture.SampleLevel(samplerState, ClampPixel(currPixelPosition + uint2(-1, 0), textureSize) * texelSize, 0);
    float4 colorRight  = texture.SampleLevel(samplerState, ClampPixel(currPixelPosition + uint2(1, 0), textureSize) * texelSize, 0);

    // Luminance calculation (perceived brightness)
    float lumCenter = dot(colorCenter.rgb, float3(0.299, 0.587, 0.114));
    float lumUp     = dot(colorUp.rgb, float3(0.299, 0.587, 0.114));
    float lumDown   = dot(colorDown.rgb, float3(0.299, 0.587, 0.114));
    float lumLeft   = dot(colorLeft.rgb, float3(0.299, 0.587, 0.114));
    float lumRight  = dot(colorRight.rgb, float3(0.299, 0.587, 0.114));

    // Compute min and max luminance
    float lumMin = min(lumCenter, min(min(lumLeft, lumRight), min(lumUp, lumDown)));
    float lumMax = max(lumCenter, max(max(lumLeft, lumRight), max(lumUp, lumDown)));

    // Compute gradients for edge direction
    float gradientHorizontal = abs(lumLeft - lumRight);
    float gradientVertical   = abs(lumUp - lumDown);
    bool isHorizontal = (gradientHorizontal >= gradientVertical);

    // Adjust UV based on edge direction
    float2 offset = isHorizontal ? float2(texelSize.x, 0.0) : float2(0.0, texelSize.y);

    // Compute clamped pixel positions for additional samples
    float2 uvCenter = currPixelPosition * texelSize;
    float2 uvOffsetPos = (ClampPixel(currPixelPosition + (isHorizontal ? uint2(1, 0) : uint2(0, 1)), textureSize)) * texelSize;
    float2 uvOffsetNeg = (ClampPixel(currPixelPosition - (isHorizontal ? uint2(1, 0) : uint2(0, 1)), textureSize)) * texelSize;

    float4 sample1 = texture.SampleLevel(samplerState, lerp(uvCenter, uvOffsetPos, 0.5), 0);
    float4 sample2 = texture.SampleLevel(samplerState, lerp(uvCenter, uvOffsetNeg, 0.5), 0);

    float3 finalColor = (sample1.rgb + sample2.rgb) * 0.5;

    // Blend color if within luminance range
    float finalLum = dot(finalColor, float3(0.299, 0.587, 0.114));
    if (finalLum < lumMin || finalLum > lumMax) {
        finalColor = colorCenter.rgb;
    }

    return float4(finalColor, colorCenter.a);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float4> input = ResourceDescriptorHeap[Inputs.InputID];
    RWTexture2D<float4> output = ResourceDescriptorHeap[Inputs.OutputID];
    SamplerState sampler = SamplerDescriptorHeap[Inputs.SamplerID];

    int width, height;
    output.GetDimensions(width, height);
    if (tid.x > width || tid.y > height) return;

    float4 color = ApplyFXAA(input, sampler, tid.xy, 1.0 / float2(width, height), uint2(width, height));
    output[tid.xy] = color;
}
