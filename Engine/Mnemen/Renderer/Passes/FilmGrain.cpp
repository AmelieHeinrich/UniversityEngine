//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:00:29
//

#include "FilmGrain.hpp"

FilmGrain::FilmGrain(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle computeShader  = AssetManager::Get("Assets/Shaders/FilmGrain/Compute.hlsl", AssetType::Shader);
    auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    mPipeline = mRHI->CreateComputePipeline(computeShader->Shader, signature);
}

void FilmGrain::Render(const Frame& frame, ::Ref<Scene> scene)
{   
    CameraComponent* camera = scene->GetMainCamera();
    if (!camera)
        return;
    if (!camera->Volume)
        return;
    if (!camera->Volume->Volume.EnableFilmGrain)
        return;

    auto color = RendererTools::Get("LDRColorBuffer");

    struct {
        int InputIndex;
        int DepthIndex;
        float amount;
        float osg_FrameTime;
    } PushConstants = {
        color->Descriptor(ViewType::Storage),
        0,
        camera->Volume->Volume.FilmGrainAmount,
        mTimer.GetElapsed(),
    };
    
    // Your settings are in camera->Volume->Volume
    //

    frame.CommandBuffer->BeginMarker("Film Grain");
    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mPipeline);
    frame.CommandBuffer->ComputePushConstants(&PushConstants, sizeof(PushConstants), 0);
    frame.CommandBuffer->Dispatch(frame.Width / 8, frame.Height / 8, 1);
    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::Common);
    frame.CommandBuffer->EndMarker();

    mTimer.Restart();
}
