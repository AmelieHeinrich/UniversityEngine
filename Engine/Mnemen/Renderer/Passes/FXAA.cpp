//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 14:13:28
//

#include "FXAA.hpp"

#include <Core/Application.hpp>
#include <Core/Profiler.hpp>

FXAA::FXAA(RHI::Ref rhi)
    : RenderPass(rhi)
{
    int width, height;
    Application::Get()->GetWindow()->PollSize(width, height);

    // Create Pipeline
    Asset::Handle computeShader = AssetManager::Get("Assets/Shaders/FXAA/Compute.hlsl", AssetType::Shader);
    auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    mPipeline = mRHI->CreateComputePipeline(computeShader->Shader, signature);

    // Create texture
    TextureDesc tempTextureDesc;
    tempTextureDesc.Width = width;
    tempTextureDesc.Height = height;
    tempTextureDesc.Levels = 1;
    tempTextureDesc.Depth = 1;
    tempTextureDesc.Format = TextureFormat::RGBA16Float;
    tempTextureDesc.Name = "FXAA Temporary Texture";
    tempTextureDesc.Usage = TextureUsage::Storage;

    auto temp = RendererTools::CreateSharedTexture("FXAATemporaryTexture", tempTextureDesc);
    temp->AddView(ViewType::Storage);

    RendererTools::CreateSharedSampler("FXAASampler", SamplerFilter::Linear, SamplerAddress::Wrap);
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
    auto output = RendererTools::Get("FXAATemporaryTexture");
    auto sampler = RendererTools::Get("FXAASampler");

    frame.CommandBuffer->BeginMarker("FXAA (Fast Approximate Anti-Aliasing)");
    
    // Apply FXAA
    {
        PROFILE_SCOPE("FXAA::ApplyFXAA");
        struct {
            int InputID;
            int OutputID;
            int SamplerID;
            int Pad;
        } PushConstants = {
            hdr->Descriptor(ViewType::ShaderResource),
            output->Descriptor(ViewType::Storage),
            sampler->Descriptor(),
            0
        };

        frame.CommandBuffer->BeginMarker("Apply FXAA");
        frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(output->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->SetComputePipeline(mPipeline);
        frame.CommandBuffer->ComputePushConstants(&PushConstants, sizeof(PushConstants), 0);
        frame.CommandBuffer->Dispatch(frame.Width / 7, frame.Height / 7, 1);
        frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Common);
        frame.CommandBuffer->UAVBarrier(output->Texture);
        frame.CommandBuffer->EndMarker();
    }

    // Copy temporary to output
    {
        PROFILE_SCOPE("FXAA::CopyOutput");
        frame.CommandBuffer->BeginMarker("Copy Output");
        frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::CopyDest);
        frame.CommandBuffer->Barrier(output->Texture, ResourceLayout::CopySource);
        frame.CommandBuffer->CopyTextureToTexture(hdr->Texture, output->Texture);
        frame.CommandBuffer->Barrier(output->Texture, ResourceLayout::Common);
        frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Common);
        frame.CommandBuffer->EndMarker();
    }

    frame.CommandBuffer->EndMarker();
}
