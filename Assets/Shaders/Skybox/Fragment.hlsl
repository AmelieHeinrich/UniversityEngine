//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 18:00:17
//

struct FragmentIn
{
    float4 Position : SV_POSITION;
    float3 LocalPosition : COLOR0;
};

struct Constants
{
    uint EnvironmentMap;
    uint CubeSampler;
    column_major float4x4 ModelViewProjection;
};

ConstantBuffer<Constants> Settings : register(b0);

float4 PSMain(FragmentIn input) : SV_Target
{
    TextureCube EnvironmentMap = ResourceDescriptorHeap[Settings.EnvironmentMap];
    SamplerState CubeSampler = SamplerDescriptorHeap[Settings.CubeSampler];

    float3 env_vector = normalize(input.LocalPosition);
    float4 color = EnvironmentMap.SampleLevel(CubeSampler, env_vector, 0);

    return color;
}
