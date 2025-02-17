//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 22:01:27
//

#include "Deferred.hpp"

#include <Asset/AssetManager.hpp>
#include <Core/Application.hpp>
#include <Core/Profiler.hpp>
#include <Core/Statistics.hpp>

Deferred::Deferred(RHI::Ref rhi)
    : RenderPass(rhi)
{
    int width, height;
    Application::Get()->GetWindow()->PollSize(width, height);

    // BRDF
    {
        TextureDesc desc = {};
        desc.Width = 512;
        desc.Height = 512;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "BRDF Texture";
        desc.Usage = TextureUsage::ShaderResource | TextureUsage::Storage;
        desc.Format = TextureFormat::RG16Float;

        auto brdf = RendererTools::CreateSharedTexture("BRDF", desc);
        brdf->AddView(ViewType::ShaderResource);
        brdf->AddView(ViewType::Storage);
    }

    // Color buffer
    {
        TextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "Main Color Buffer";
        desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
        desc.Format = TextureFormat::RGBA16Float;
       
        auto renderTarget = RendererTools::CreateSharedTexture("HDRColorBuffer", desc);
        renderTarget->AddView(ViewType::RenderTarget);
        renderTarget->AddView(ViewType::ShaderResource);
        renderTarget->AddView(ViewType::Storage);
    }

    // BRDF pipeline
    {
        Asset::Handle brdfShader = AssetManager::Get("Assets/Shaders/BRDF/Compute.hlsl", AssetType::Shader);
        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
        mBRDFPipeline = mRHI->CreateComputePipeline(brdfShader->Shader, signature);
    }

    // Accumulation Pipeline
    {
        Asset::Handle lightShader = AssetManager::Get("Assets/Shaders/Deferred/LightAccumulationCompute.hlsl", AssetType::Shader);
        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
        mLightPipeline = mRHI->CreateComputePipeline(lightShader->Shader, signature);
    }

    // Compute BRDF and stuff!!
    {
        struct Data {
            int lut;
            glm::vec3 pad;
        } data = {
            RendererTools::Get("BRDF")->Descriptor(ViewType::Storage),
            glm::vec3(0)
        };

        auto cmdBuffer = mRHI->CreateCommandBuffer(true);
        cmdBuffer->Begin();
        cmdBuffer->SetComputePipeline(mBRDFPipeline);
        cmdBuffer->ComputePushConstants(&data, sizeof(data), 0);
        cmdBuffer->Dispatch(512 / 32, 512 / 32, 1);
        cmdBuffer->End();
        mRHI->Submit({ cmdBuffer });
        mRHI->Wait();
    }
}

void Deferred::Render(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    auto cameraBuffer = RendererTools::Get("CameraRingBuffer");
    auto sampler = RendererTools::Get("MaterialSampler");

    auto depthBuffer = RendererTools::Get("GBufferDepth");
    auto normalBuffer = RendererTools::Get("GBufferNormal");
    auto albedoBuffer = RendererTools::Get("GBufferAlbedo");
    auto colorBuffer = RendererTools::Get("HDRColorBuffer");

    struct PushConstants {
        int Depth;
        int Albedo;
        int Normal;
        int Output;
    } data = {
        depthBuffer->Descriptor(ViewType::ShaderResource),
        albedoBuffer->Descriptor(ViewType::ShaderResource),
        normalBuffer->Descriptor(ViewType::ShaderResource),
        colorBuffer->Descriptor(ViewType::Storage)
    };

    frame.CommandBuffer->BeginMarker("Light Accumulation");
    frame.CommandBuffer->Barrier(colorBuffer->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mLightPipeline);
    frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
    frame.CommandBuffer->Dispatch(frame.Width / 7, frame.Height / 7, 1);
    frame.CommandBuffer->UAVBarrier(colorBuffer->Texture);
    frame.CommandBuffer->EndMarker();
}
