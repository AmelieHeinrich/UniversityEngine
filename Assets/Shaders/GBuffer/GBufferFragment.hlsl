//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 22:11:15
//

struct MeshInput
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 UV : TEXCOORD;
    uint MeshletIndex : COLOR0;
};

uint hash(uint a)
{
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}

struct PushConstants
{
    int Matrices;
    int VertexBuffer;
    int IndexBuffer;
    int MeshletBuffer;

    int MeshletVertices;
    int MeshletTriangleBuffer;
    int AlbedoTexture;
    int NormalTexture;

    int PBRTexture;
    int LinearSampler;
    int ShowMeshlets;
    int Padding;

    column_major float4x4 Transform;
    column_major float4x4 InvTransform;
};

ConstantBuffer<PushConstants> Constants : register(b0);

struct GBufferOutput
{
    float4 Normal : SV_Target0;
    float4 Albedo : SV_Target1;
    float2 PBR : SV_Target2;
};

float3 GetNormalFromMap(MeshInput Input)
{
    Texture2D NormalTexture = ResourceDescriptorHeap[Constants.NormalTexture];
    SamplerState Sampler = SamplerDescriptorHeap[Constants.LinearSampler];

    float3 tangentNormal = NormalTexture.Sample(Sampler, Input.UV.xy).rgb * 2.0 - 1.0;
    float3x3 TBN = float3x3(Input.Tangent, Input.Bitangent, Input.Normal);

    return normalize(mul(tangentNormal, TBN));
}

GBufferOutput PSMain(MeshInput input)
{
    Texture2D<float4> albedoTexture = ResourceDescriptorHeap[Constants.AlbedoTexture];
    Texture2D<float4> pbrTexture = ResourceDescriptorHeap[Constants.PBRTexture];
    SamplerState linearSampler = SamplerDescriptorHeap[Constants.LinearSampler];

    uint meshletHash = hash(input.MeshletIndex);
    float3 meshletColor = float3(float(meshletHash & 255), float((meshletHash >> 8) & 255), float((meshletHash >> 16) & 255)) / 255.0;

    float4 textureColor = albedoTexture.Sample(linearSampler, input.UV);
    if (textureColor.a < 0.25)
        discard;
    textureColor.rgb = pow(textureColor.rgb, 2.2);

    float3 normal = normalize(input.Normal);
    if (Constants.NormalTexture != -1) {
        normal = GetNormalFromMap(input);
    }

    float3 pbr = pbrTexture.Sample(linearSampler, input.UV).rgb;

    GBufferOutput output;
    output.Albedo = Constants.ShowMeshlets ? float4(meshletColor, 1.0) : textureColor;
    output.Normal = float4(normal, 1.0);
    output.PBR = float2(pbr.b, pbr.g);
    return output;
}
