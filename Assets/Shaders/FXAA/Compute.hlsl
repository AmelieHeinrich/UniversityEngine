//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 14:17:05
//

struct Data
{
    int OutputIndex;
};

ConstantBuffer<Data> Constants : register(b0);

float4 ApplyFXAA(RWTexture2D<float4> texture, uint2 currPixelPosition)
{
    // Sample colors at the current pixel and its neighbors
    float4 colorCenter = texture[currPixelPosition];
    float4 colorUp     = texture[currPixelPosition + uint2(0, -1)];
    float4 colorDown   = texture[currPixelPosition + uint2(0, 1)];
    float4 colorLeft   = texture[currPixelPosition + uint2(-1, 0)];
    float4 colorRight  = texture[currPixelPosition + uint2(1, 0)];

    // Luminance calculation for each sampled pixel (perceived brightness)
    float lumCenter = dot(colorCenter.rgb, float3(0.299, 0.587, 0.114));
    float lumUp     = dot(colorUp.rgb, float3(0.299, 0.587, 0.114));
    float lumDown   = dot(colorDown.rgb, float3(0.299, 0.587, 0.114));
    float lumLeft   = dot(colorLeft.rgb, float3(0.299, 0.587, 0.114));
    float lumRight  = dot(colorRight.rgb, float3(0.299, 0.587, 0.114));

    // Compute minimum and maximum luminance in the neighborhood
    float lumMin = min(lumCenter, min(min(lumLeft, lumRight), min(lumUp, lumDown)));
    float lumMax = max(lumCenter, max(max(lumLeft, lumRight), max(lumUp, lumDown)));

    // Calculate edge detection gradient
    float gradientHorizontal = abs(lumLeft - lumRight);
    float gradientVertical   = abs(lumUp - lumDown);

    // Determine edge direction (horizontal or vertical)
    bool isHorizontal = (gradientHorizontal >= gradientVertical);

    // Adjust UV coordinates based on edge direction
    int2 offset = isHorizontal ? int2(1, 0) : int2(0, 1);

    float4 sample1 = texture[currPixelPosition + offset / 2];
    float4 sample2 = texture[currPixelPosition - offset / 2];

    float3 finalColor = (sample1.rgb + sample2.rgb) * 0.5;

    // Blend the color if it's within the luminance range
    float finalLum = dot(finalColor, float3(0.299, 0.587, 0.114));
    if (finalLum < lumMin || finalLum > lumMax) {
        finalColor = colorCenter.rgb;
    }

    return float4(finalColor, colorCenter.a);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    RWTexture2D<float4> Output = ResourceDescriptorHeap[Constants.OutputIndex];

    int width, height;
    Output.GetDimensions(width, height);
    if (ThreadID.x >= width || ThreadID.y >= height)
        return;

    float4 color = ApplyFXAA(Output, ThreadID.xy);
    Output[ThreadID.xy] = color;
}
