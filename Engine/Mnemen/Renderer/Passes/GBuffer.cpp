//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 10:11:48
//

#include "GBuffer.hpp"

#include <Asset/AssetManager.hpp>
#include <Core/Application.hpp>
#include <Core/Profiler.hpp>
#include <Core/Statistics.hpp>

GBuffer::GBuffer(RHI::Ref rhi)
    : RenderPass(rhi)
{
    int width, height;
    Application::Get()->GetWindow()->PollSize(width, height);

    Asset::Handle gbufferShaderTask = AssetManager::Get("Assets/Shaders/GBuffer/GBufferAmplification.hlsl", AssetType::Shader);
    Asset::Handle gbufferShaderIn = AssetManager::Get("Assets/Shaders/GBuffer/GBufferMesh.hlsl", AssetType::Shader);
    Asset::Handle gbufferShaderOut = AssetManager::Get("Assets/Shaders/GBuffer/GBufferFragment.hlsl", AssetType::Shader);

    // Depth buffer
    {
        TextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "GBuffer - Depth";
        desc.Usage = TextureUsage::DepthTarget;
        desc.Format = TextureFormat::Depth32;
       
        auto depthBuffer = RendererTools::CreateSharedTexture("GBufferDepth", desc);
        depthBuffer->AddView(ViewType::DepthTarget);
        depthBuffer->AddView(ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
    }

    // Normal buffer
    {
        TextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "GBuffer - Normals";
        desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
        desc.Format = TextureFormat::RGBA16Float;
       
        auto renderTarget = RendererTools::CreateSharedTexture("GBufferNormal", desc);
        renderTarget->AddView(ViewType::RenderTarget);
        renderTarget->AddView(ViewType::ShaderResource);
        renderTarget->AddView(ViewType::Storage);
    }

    // Albedo buffer
    {
        TextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "GBuffer - Albedo";
        desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
        desc.Format = TextureFormat::RGBA8;
       
        auto renderTarget = RendererTools::CreateSharedTexture("GBufferAlbedo", desc);
        renderTarget->AddView(ViewType::RenderTarget);
        renderTarget->AddView(ViewType::ShaderResource);
        renderTarget->AddView(ViewType::Storage);
    }

    // PBR buffer
    {
        TextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "GBuffer - PBR";
        desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
        desc.Format = TextureFormat::RG8;
       
        auto renderTarget = RendererTools::CreateSharedTexture("GBufferPBR", desc);
        renderTarget->AddView(ViewType::RenderTarget);
        renderTarget->AddView(ViewType::ShaderResource);
        renderTarget->AddView(ViewType::Storage);
    }

    // GBuffer Pipeline
    {
        GraphicsPipelineSpecs specs = {};
        specs.Bytecodes[ShaderType::Amplification] = gbufferShaderTask->Shader;
        specs.Bytecodes[ShaderType::Mesh] = gbufferShaderIn->Shader;
        specs.Bytecodes[ShaderType::Fragment] = gbufferShaderOut->Shader;
        specs.Formats.push_back(TextureFormat::RGBA16Float);
        specs.Formats.push_back(TextureFormat::RGBA8);
        specs.Formats.push_back(TextureFormat::RG8);
        specs.Cull = CullMode::None;
        specs.Fill = FillMode::Solid;
        specs.Depth = DepthOperation::Less;
        specs.DepthEnabled = true;
        specs.DepthFormat = TextureFormat::Depth32;
        specs.CCW = false;
        specs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 12 + (sizeof(glm::mat4) * 2));
        specs.UseAmplification = true;

        mPipeline = mRHI->CreateMeshPipeline(specs);
    }

    RendererTools::CreateSharedRingBuffer("CameraRingBuffer", 512);
    RendererTools::CreateSharedSampler("MaterialSampler", SamplerFilter::Linear, SamplerAddress::Wrap, true);
}

void GBuffer::Render(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    auto cameraBuffer = RendererTools::Get("CameraRingBuffer");
    auto sampler = RendererTools::Get("MaterialSampler");

    auto depthBuffer = RendererTools::Get("GBufferDepth");
    auto normalBuffer = RendererTools::Get("GBufferNormal");
    auto albedoBuffer = RendererTools::Get("GBufferAlbedo");
    auto pbrBuffer = RendererTools::Get("GBufferPBR");
    auto whiteTexture = RendererTools::Get("WhiteTexture");
    auto blackTexture = RendererTools::Get("BlackTexture");

    CameraComponent* camera = {};
    if (scene) {
        camera = scene->GetMainCamera();
    }
    glm::mat4 matrices[] = {
        camera->View,
        camera->Projection
    };
    cameraBuffer->RBuffer[frame.FrameIndex]->CopyMapped(matrices, sizeof(matrices));

    frame.CommandBuffer->BeginMarker("GBuffer");
    frame.CommandBuffer->Barrier(albedoBuffer->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(normalBuffer->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(pbrBuffer->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(depthBuffer->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->SetViewport(0, 0, frame.Width, frame.Height);
    frame.CommandBuffer->SetRenderTargets({ normalBuffer->GetView(ViewType::RenderTarget), albedoBuffer->GetView(ViewType::RenderTarget), pbrBuffer->GetView(ViewType::RenderTarget) }, depthBuffer->GetView(ViewType::DepthTarget));
    frame.CommandBuffer->ClearRenderTarget(normalBuffer->GetView(ViewType::RenderTarget), 0.0f, 0.0f, 0.0f);
    frame.CommandBuffer->ClearRenderTarget(albedoBuffer->GetView(ViewType::RenderTarget), 0.0f, 0.0f, 0.0f);
    frame.CommandBuffer->ClearRenderTarget(pbrBuffer->GetView(ViewType::RenderTarget), 0.0f, 0.0f, 0.0f);
    frame.CommandBuffer->ClearDepth(depthBuffer->GetView(ViewType::DepthTarget));
    frame.CommandBuffer->SetMeshPipeline(mPipeline);

    // Draw function for each model
    std::function<void(Frame frame, MeshNode*, Mesh* model, glm::mat4 transform, MaterialComponent* material)> drawNode = [&](Frame frame, MeshNode* node, Mesh* model, glm::mat4 transform, MaterialComponent* material) {
        if (!node) {
            return;
        }

        for (MeshPrimitive primitive : node->Primitives) {
            if (!primitive.IsBoxInFrustum(transform, camera->View, camera->Projection))
                continue;
            Statistics::Get().InstanceCount++;
            MeshMaterial meshMaterial = model->Materials[primitive.MaterialIndex];

            // NOTE(ame): Ugly disgusting piece of shit code but it'll do the trick. Yippee!!!
            int albedoIndex = whiteTexture->Descriptor(ViewType::ShaderResource);
            int normalIndex = whiteTexture->Descriptor(ViewType::ShaderResource);
            int pbrIndex = whiteTexture->Descriptor(ViewType::ShaderResource);
            if (material) {
                if (material->InheritFromModel) {
                    albedoIndex = meshMaterial.Albedo ? meshMaterial.AlbedoView->GetDescriptor().Index : whiteTexture->Descriptor(ViewType::ShaderResource);
                    normalIndex = meshMaterial.Normal ? meshMaterial.NormalView->GetDescriptor().Index : -1;
                    pbrIndex = meshMaterial.PBR ? meshMaterial.PBRView->GetDescriptor().Index : blackTexture->Descriptor(ViewType::ShaderResource);
                } else {
                    albedoIndex = material->Albedo ? material->Albedo->ShaderView->GetDescriptor().Index : whiteTexture->Descriptor(ViewType::ShaderResource);
                    normalIndex = material->Normal ? material->Normal->ShaderView->GetDescriptor().Index : -1;
                    pbrIndex = material->PBR ? material->PBR->ShaderView->GetDescriptor().Index : -1;
                }
            } else {
                albedoIndex = meshMaterial.Albedo ? meshMaterial.AlbedoView->GetDescriptor().Index : whiteTexture->Descriptor(ViewType::ShaderResource);
                normalIndex = meshMaterial.Normal ? meshMaterial.NormalView->GetDescriptor().Index : -1;
                pbrIndex = meshMaterial.PBR ? meshMaterial.PBRView->GetDescriptor().Index : -1;
            }

            struct PushConstants {
                int Matrices;
                int VertexBuffer;
                int IndexBuffer;
                int MeshletBuffer;
                int MeshletVertices;
                int MeshletTriangleBuffer;
                
                int Albedo;
                int Normal;
                int PBR;
                int Sampler;
                int ShowMeshlets;
                
                int Padding;
                glm::mat4 Transform;
                glm::mat4 InvTransform;
            } data = {
                cameraBuffer->Descriptor(ViewType::None, frame.FrameIndex),
                primitive.VertexBuffer->SRV(),
                primitive.IndexBuffer->SRV(),
                primitive.MeshletBuffer->SRV(),
                primitive.MeshletVertices->SRV(),
                primitive.MeshletTriangles->SRV(),
                albedoIndex,
                normalIndex,
                pbrIndex,
                sampler->Descriptor(),
                camera->Volume->Volume.VisualizeMeshlets,
                0,
                
                transform,
                glm::inverse(transform)
            };
            UInt32 threadGroupCountX = static_cast<UInt32>((primitive.MeshletCount / 32) + 1);

            frame.CommandBuffer->Barrier(primitive.VertexBuffer, ResourceLayout::Shader);
            frame.CommandBuffer->Barrier(primitive.IndexBuffer, ResourceLayout::Shader);
            frame.CommandBuffer->Barrier(primitive.MeshletBuffer, ResourceLayout::Shader);
            frame.CommandBuffer->Barrier(primitive.MeshletVertices, ResourceLayout::Shader);
            frame.CommandBuffer->Barrier(primitive.MeshletTriangles, ResourceLayout::Shader);
            frame.CommandBuffer->GraphicsPushConstants(&data, sizeof(data), 0);
            frame.CommandBuffer->DispatchMesh(threadGroupCountX, primitive.IndexCount / 3);
            frame.CommandBuffer->Barrier(primitive.VertexBuffer, ResourceLayout::Common);
            frame.CommandBuffer->Barrier(primitive.IndexBuffer, ResourceLayout::Common);
            frame.CommandBuffer->Barrier(primitive.MeshletBuffer, ResourceLayout::Common);
            frame.CommandBuffer->Barrier(primitive.MeshletVertices, ResourceLayout::Common);
            frame.CommandBuffer->Barrier(primitive.MeshletTriangles, ResourceLayout::Common);
        }
        if (!node->Children.empty()) {
            for (MeshNode* child : node->Children) {
                drawNode(frame, child, model, transform, material);
            }
        }
    };

    if (scene) {
        auto registry = scene->GetRegistry();
        auto view = registry->view<TransformComponent, MeshComponent>();
        for (auto [id, transform, mesh] : view.each()) {
            Entity entity(registry);
            entity.ID = id;
            if (mesh.Loaded) {
                MaterialComponent* component = nullptr;
                if (entity.HasComponent<MaterialComponent>()) {
                    component = &entity.GetComponent<MaterialComponent>();
                }

                drawNode(frame, mesh.MeshAsset->Mesh.Root, &mesh.MeshAsset->Mesh, entity.GetWorldTransform(), component);
            }
        }
    }
    frame.CommandBuffer->Barrier(albedoBuffer->Texture, ResourceLayout::Shader);
    frame.CommandBuffer->Barrier(normalBuffer->Texture, ResourceLayout::Shader);
    frame.CommandBuffer->Barrier(depthBuffer->Texture, ResourceLayout::Shader);
    frame.CommandBuffer->EndMarker();
}
