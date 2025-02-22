//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:02:20
//

struct Parameters
{   
    uint InputIndex;
    uint DepthIndex;

    float amount;
    float osg_FrameTime;
    
};

ConstantBuffer<Parameters> Settings : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    RWTexture2D<float4> Output = ResourceDescriptorHeap[Settings.InputIndex];

    float toRadians = 3.14 / 180;
    float4 color = Output.Load(ThreadID.xy);

    int width, height;

    float randomIntensity =
        frac(10000 * sin
            (ThreadID.x + ThreadID.y * Settings.osg_FrameTime) 
            * toRadians);
    
    color.rgb += Settings.amount * randomIntensity;
    Output[ThreadID.xy] = color;
}
