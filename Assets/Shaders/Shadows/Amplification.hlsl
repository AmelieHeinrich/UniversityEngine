//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-20 16:00:57
//

struct Payload
{
    uint MeshletIndices[32];
};

struct PushConstants
{
    int VertexBuffer;
    int IndexBuffer;
    int MeshletBuffer;
    int MeshletVertices;
    int MeshletTriangleBuffer;
    int3 Pad;

    column_major float4x4 Transform;
    column_major float4x4 LightView;
    column_major float4x4 LightProj;
};

ConstantBuffer<PushConstants> Constants : register(b0);

groupshared Payload s_Payload;

[numthreads(32, 1, 1)]
void ASMain(uint gtid : SV_GroupThreadID, uint dtid : SV_DispatchThreadID, uint gid : SV_GroupID)
{
    s_Payload.MeshletIndices[gtid] = dtid;
    DispatchMesh(32, 1, 1, s_Payload);
}
