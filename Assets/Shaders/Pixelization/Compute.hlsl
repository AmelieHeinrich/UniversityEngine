//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:02:20
//

struct Parameters
{
    int Placeholder; // Delete this if you add any settings...
};

ConstantBuffer<Parameters> Settings : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
    // Code goes here
}
