//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 11:49:11
//

static const int SHADOW_CASCADE_COUNT = 4;
static const int SHADOW_PCF_KERNELS[SHADOW_CASCADE_COUNT] = {
    1, 1, 1, 1
}; 

struct Cascade
{
    int SRVIndex;
    float Split;
    int2 Pad;
    column_major float4x4 View;
    column_major float4x4 Proj;
};

struct CascadeBuffer
{
    Cascade Cascades[SHADOW_CASCADE_COUNT];
};

float4 GetCascadeColor(int layer)
{
    switch (layer) {
        case 0:
            return float4(1.0f, 0.0f, 0.0f, 1.0f);
        case 1:
            return float4(0.0f, 1.0f, 0.0f, 1.0f);
        case 2:
            return float4(0.0f, 0.0f, 1.0f, 1.0f);
        case 3:
            return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    return 0.0f;
}
