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
    
    RendererTools::CreateSharedSampler("CubemapSampler", SamplerFilter::Linear, SamplerAddress::Clamp, true);
    RendererTools::CreateSharedSampler("ShadowSampler", SamplerFilter::Nearest, SamplerAddress::Clamp, false, true);
    RendererTools::CreateSharedRingBuffer("DeferredCameraBuffer", 512);

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
        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 16 + sizeof(glm::mat4));
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

    CameraComponent* mainCamera = scene->GetMainCamera();
    if (!mainCamera)
        return;
    if (!mainCamera->Volume)
        return;

    auto sampler = RendererTools::Get("MaterialSampler");
    auto depthBuffer = RendererTools::Get("GBufferDepth");
    auto normalBuffer = RendererTools::Get("GBufferNormal");
    auto albedoBuffer = RendererTools::Get("GBufferAlbedo");
    auto pbrBuffer = RendererTools::Get("GBufferPBR");
    auto colorBuffer = RendererTools::Get("HDRColorBuffer");
    auto brdf = RendererTools::Get("BRDF");
    auto cubeSampler = RendererTools::Get("CubemapSampler");
    auto shadowSampler = RendererTools::Get("ShadowSampler");
    auto lightBuffer = RendererTools::Get("LightBuffer");
    auto cameraBuffer = RendererTools::Get("DeferredCameraBuffer");
    auto cascadeBuffer = RendererTools::Get("CascadeBuffer");

    struct CameraData {
        glm::mat4 InverseViewProj;
        glm::mat4 Projection;

        glm::vec3 Position;
        float Pad;

        float Near;
        float Far;
        glm::vec2 Pad1;
    } cam = {
        glm::inverse(mainCamera->Projection * mainCamera->View),
        mainCamera->Projection,
        mainCamera->Position,
        0,
        mainCamera->Near,
        mainCamera->Far,
        glm::vec2(0)
    };
    cameraBuffer->RBuffer[frame.FrameIndex]->CopyMapped(&cam, sizeof(cam));

    struct PushConstants {
        int Depth;
        int Albedo;
        int Normal;
        int Output;

        int PBR;
        int Irradiance;
        int Prefilter;
        int BRDF;

        int sampler;
        int regularSampler;
        int lightBuffer;
        int cascadeBuffer;

        int shadowSampler;
        int cameraBuffer;
        float direct;
        float indirect;
    } data = {
        depthBuffer->Descriptor(ViewType::ShaderResource),
        albedoBuffer->Descriptor(ViewType::ShaderResource),
        normalBuffer->Descriptor(ViewType::ShaderResource),
        colorBuffer->Descriptor(ViewType::Storage),

        pbrBuffer->Descriptor(ViewType::ShaderResource),
        scene->GetSkybox()->IrradianceMapSRV->GetDescriptor().Index,
        scene->GetSkybox()->PrefilterMapSRV->GetDescriptor().Index,
        brdf->Descriptor(ViewType::ShaderResource),

        cubeSampler->Descriptor(),
        sampler->Descriptor(),
        lightBuffer->Descriptor(ViewType::None, frame.FrameIndex),
        cascadeBuffer->Descriptor(ViewType::None, frame.FrameIndex),

        shadowSampler->Descriptor(),
        cameraBuffer->Descriptor(ViewType::None, frame.FrameIndex),
        mainCamera->Volume->Volume.DirectLight,
        mainCamera->Volume->Volume.IndirectLight
    };

    frame.CommandBuffer->BeginMarker("Light Accumulation");
    frame.CommandBuffer->Barrier(colorBuffer->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mLightPipeline);
    frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
    frame.CommandBuffer->Dispatch(frame.Width / 7, frame.Height / 7, 1);
    frame.CommandBuffer->UAVBarrier(colorBuffer->Texture);
    frame.CommandBuffer->EndMarker();
}
