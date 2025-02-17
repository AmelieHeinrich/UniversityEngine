//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 23:15:31
//

struct LightData
{
    int DirLightSRV;
    int DirLightCount;
    int PointLightSRV;
    int PointLightCount;
    int SpotLightSRV;
    int SpotLightCount;
};

struct DirectionalLight
{
    float3 Direction;
    float Strength;

    float3 Color;
    bool CastShadows;
};

struct PointLight
{
    float3 Color;
    float Radius;
    float3 Position;
    float Pad;
};

struct SpotLight
{
    float Radius;
    float OuterRadius;
    bool CastShadows;
    int ShadowMap;

    float3 Color;
    int Pad;

    float3 Direction;
    float Pad1;

    column_major float4x4 LightView;
    column_major float4x4 LightProj;
};
