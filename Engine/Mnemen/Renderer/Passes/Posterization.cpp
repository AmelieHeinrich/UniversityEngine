//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:00:29
//

#include "Posterization.hpp"
#include <imgui.h>

Posterization::Posterization(RHI::Ref rhi)
    : RenderPass(rhi)
{
    //retrieve the' computer shader from the asset manager
    Asset::Handle computerShader = AssetManager::Get("Assets/Shaders/Posterization/Compute.hlsl", AssetType::Shader); 

    //Create a root signature for the shader (push constants for small data)
    auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 2);

    //Create the compute pipeline with the shader and root signature 
    mPipeline = mRHI->CreateComputePipeline(computerShader->Shader, signature);
}

void Posterization::Render(const Frame& frame, ::Ref<Scene> scene)
{
    CameraComponent* camera = scene->GetMainCamera();
    if (!camera)
        return;
    if (!camera->Volume)
        return;
    if (!camera->Volume->Volume.EnablePosterization)
        return;

    auto color = RendererTools::Get("HDRColorBuffer");
    struct Data {
        int TextureIndex;
        float Levels;
    } PushConstants = {
        color->Descriptor(ViewType::Storage), 
        camera->Volume->Volume.PosterizationLevels
    };

    // Your settings are in camera->Volume->Volume
    //
    float levels = camera->Volume->Volume.PosterizationLevels;

    frame.CommandBuffer->BeginMarker("Posterization");
    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mPipeline);
    frame.CommandBuffer->ComputePushConstants(&PushConstants, sizeof (PushConstants), 0);
    frame.CommandBuffer->Dispatch(frame.Width / 8, frame.Height / 8, 1);
    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::Common);
    frame.CommandBuffer->EndMarker();

}
