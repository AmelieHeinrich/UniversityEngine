//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-20 15:34:00
//

struct Payload
{
    uint MeshletIndices[32];
};

struct CameraMatrices
{
    column_major float4x4 View;
    column_major float4x4 Projection;
    float4 Planes[6];
};

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

groupshared Payload s_Payload;

[numthreads(32, 1, 1)]
void ASMain(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{
    s_Payload.MeshletIndices[gtid] = dtid;
    DispatchMesh(32, 1, 1, s_Payload);
}
