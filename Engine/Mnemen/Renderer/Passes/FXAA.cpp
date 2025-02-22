//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 14:13:28
//

#include "FXAA.hpp"

#include <Core/Profiler.hpp>

FXAA::FXAA(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle computeShader = AssetManager::Get("Assets/Shaders/FXAA/Compute.hlsl", AssetType::Shader);
    auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 1);
    mPipeline = mRHI->CreateComputePipeline(computeShader->Shader, signature);
}

void FXAA::Render(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    CameraComponent* camera = scene->GetMainCamera();
    if (!camera)
        return;
    if (!camera->Volume)
        return;
    if (!camera->Volume->Volume.EnableFXAA)
        return;

    auto hdr = RendererTools::Get("HDRColorBuffer");

    struct {
        int Input;
    } PushConstants = {
        hdr->Descriptor(ViewType::Storage)
    };

    frame.CommandBuffer->BeginMarker("FXAA (Fast Approximate Anti-Aliasing)");
    frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mPipeline);
    frame.CommandBuffer->ComputePushConstants(&PushConstants, sizeof(PushConstants), 0);
    frame.CommandBuffer->Dispatch(frame.Width / 8, frame.Height / 8, 1);
    frame.CommandBuffer->UAVBarrier(hdr->Texture);
    frame.CommandBuffer->EndMarker();
}
