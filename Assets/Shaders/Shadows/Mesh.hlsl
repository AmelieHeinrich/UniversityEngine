//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-18 09:10:06
//

struct Vertex
{
    float3 Position : POSITION;
    float2 TexCoords : TEXCOORD;
    float3 Normals : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct Meshlet
{
    uint VertOffset;
    uint PrimOffset;
    uint VertCount;
    uint PrimCount;
};

struct CameraMatrices
{
    column_major float4x4 View;
    column_major float4x4 Projection;
};

struct VertexOut
{
    float4 Clip : SV_Position;
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

struct Payload
{
    uint MeshletIndices[32];
};

ConstantBuffer<PushConstants> Constants : register(b0);

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    StructuredBuffer<Vertex> Vertices = ResourceDescriptorHeap[Constants.VertexBuffer];

    // -------- //
    Vertex v = Vertices[vertexIndex];
    float4 pos = float4(v.Position, 1.0);

    VertexOut Output = (VertexOut)0;
    float4 worldPosition = mul(Constants.Transform, float4(v.Position, 1.0f));
    float4 lightViewPosition = mul(Constants.LightView, worldPosition);
    Output.Clip = mul(Constants.LightProj, lightViewPosition);
    return Output;
}

[numthreads(32, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
    uint GroupThreadID: SV_GroupThreadID,
    uint GroupID : SV_GroupID,
    in payload Payload payload,
    out indices uint3 Triangles[124],
    out vertices VertexOut Verts[64]
)
{
    StructuredBuffer<uint> Indices = ResourceDescriptorHeap[Constants.IndexBuffer];
    StructuredBuffer<Meshlet> Meshlets = ResourceDescriptorHeap[Constants.MeshletBuffer];
    StructuredBuffer<uint> MeshletVertices = ResourceDescriptorHeap[Constants.MeshletVertices];
    StructuredBuffer<uint> MeshletPrimitives = ResourceDescriptorHeap[Constants.MeshletTriangleBuffer];

    // -------- //
    uint meshletIndex = payload.MeshletIndices[GroupID];
    Meshlet m = Meshlets[meshletIndex];
    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    for (uint i = GroupThreadID; i < m.PrimCount; i += 32) {
        uint offset = m.PrimOffset + i * 3;
        Triangles[i] = uint3(
            MeshletPrimitives[offset],
            MeshletPrimitives[offset + 1],
            MeshletPrimitives[offset + 2]
        );
    }

    for (uint i = GroupThreadID; i < m.VertCount; i += 32) {
        uint index = MeshletVertices[m.VertOffset + i];
        Verts[i] = GetVertexAttributes(GroupID, index);
    }
}
