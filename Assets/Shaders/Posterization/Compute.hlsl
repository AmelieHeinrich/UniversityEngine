//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:02:20
//

struct Parameters
{
    int TextureIndex;
    int levels;  
};

ConstantBuffer<Parameters> Settings : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    RWTexture2D<float4> myTexture = ResourceDescriptorHeap[Settings.TextureIndex]; 
    
    uint2 pixelPosition = ThreadID.xy;
    float4 currentColor = myTexture.Load(pixelPosition);

    float greyscale = max(currentColor.r, max(currentColor.g, currentColor.b));
    float lower     = floor(greyscale * Settings.levels) / Settings.levels;
    float lowerDiff = abs(greyscale - lower);

    float upper     = ceil(greyscale * Settings.levels) / Settings.levels;
    float upperDiff = abs(upper - greyscale);

    float level      = lowerDiff <= upperDiff ? lower : upper;
    float adjustment = level / greyscale;
    currentColor.rgb *= adjustment;

    myTexture[ThreadID.xy] = currentColor;
}

