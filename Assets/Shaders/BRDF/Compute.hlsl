// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 19:42:09

#include "Assets/Shaders/Common/Math.hlsl"

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0 / float(NumSamples);

struct Constants
{
    uint LUT;
    uint3 Pad;
};

ConstantBuffer<Constants> Settings : register(b0);

float2 sampleHammersley(uint i)
{
    return float2(i * InvNumSamples, radicalInverse_VdC(i));
}

float3 sampleGGX(float u1, float u2, float roughness)
{
    float alpha = roughness * roughness;
    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = TwoPI * u1;
    return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX_IBL(float cosLi, float cosLo, float roughness)
{
    float r = roughness;
    float k = (r * r) / 2.0;
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

[numthreads(32, 32, 1)]
void CSMain(uint2 ThreadID : SV_DispatchThreadID)
{
    RWTexture2D<half2> LUT = ResourceDescriptorHeap[Settings.LUT];

    float outputWidth, outputHeight;
    LUT.GetDimensions(outputWidth, outputHeight);

    float cosLo = (ThreadID.x + 0.5) / outputWidth;
    float roughness = (ThreadID.y + 0.5) / outputHeight;
    cosLo = max(cosLo, Epsilon);

    float3 Lo = float3(sqrt(1.0 - cosLo * cosLo), 0.0, cosLo);

    float DFG1 = 0;
    float DFG2 = 0;

    for (uint i = 0; i < NumSamples; ++i) {
        float2 u = sampleHammersley(i);
        float3 Lh = sampleGGX(u.x, u.y, roughness);
        float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

        float cosLi = saturate(Li.z);
        float cosLh = saturate(Lh.z);
        float cosLoLh = saturate(dot(Lo, Lh));

        if (cosLi > 0.0) {
            float G = gaSchlickGGX_IBL(cosLi, cosLo, roughness);
            float Gv = G * cosLoLh / max(cosLh * cosLo, 1e-4);
            float Fc = pow(1.0 - cosLoLh, 5);
            DFG1 += (1 - Fc) * Gv;
            DFG2 += Fc * Gv;
        }
    }
    
    DFG1 /= float(NumSamples);
    DFG2 /= float(NumSamples);

    LUT[ThreadID] = float2(DFG1, DFG2);
}