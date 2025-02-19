//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-18 09:56:06
//

#include "Shadows.hpp"

#include <Utility/Math.hpp>
#include <Core/Profiler.hpp>
#include "Debug.hpp"

Shadows::Shadows(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle meshShader = AssetManager::Get("Assets/Shaders/Shadows/Mesh.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Shadows/Fragment.hlsl", AssetType::Shader);

    GraphicsPipelineSpecs specs;
    specs.Bytecodes[ShaderType::Mesh] = meshShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    specs.Cull = CullMode::Back;
    specs.DepthEnabled = true;
    specs.DepthClampEnable = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 8 + sizeof(glm::mat4) * 3);

    mCascadePipeline = rhi->CreateMeshPipeline(specs);

    // Create cascade data
    RendererTools::CreateSharedRingBuffer("CascadeBuffer", 1024);

    TextureDesc cascadeDesc = {};
    cascadeDesc.Width = DIR_LIGHT_SHADOW_DIMENSION;
    cascadeDesc.Height = DIR_LIGHT_SHADOW_DIMENSION;
    cascadeDesc.Levels = 1;
    cascadeDesc.Depth = 1;
    cascadeDesc.Format = TextureFormat::Depth32;
    cascadeDesc.Usage = TextureUsage::DepthTarget | TextureUsage::ShaderResource;
    
    cascadeDesc.Name = "Shadow Cascade 0";
    auto cascade0 = RendererTools::CreateSharedTexture("ShadowCascade0", cascadeDesc);
    cascade0->AddView(ViewType::DepthTarget);
    cascade0->AddView(ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);

    cascadeDesc.Name = "Shadow Cascade 1";
    auto cascade1 = RendererTools::CreateSharedTexture("ShadowCascade1", cascadeDesc);
    cascade1->AddView(ViewType::DepthTarget);
    cascade1->AddView(ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
    
    cascadeDesc.Name = "Shadow Cascade 2";
    auto cascade2 = RendererTools::CreateSharedTexture("ShadowCascade2", cascadeDesc);
    cascade2->AddView(ViewType::DepthTarget);
    cascade2->AddView(ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
    
    cascadeDesc.Name = "Shadow Cascade 3";
    auto cascade3 = RendererTools::CreateSharedTexture("ShadowCascade3", cascadeDesc);
    cascade3->AddView(ViewType::DepthTarget);
    cascade3->AddView(ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
}

void Shadows::Render(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();
    frame.CommandBuffer->BeginMarker("Shadows");
    RenderCascades(frame, scene);
    frame.CommandBuffer->EndMarker();
}

void Shadows::RenderCascades(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    // Select first dir light that cast shadows
    DirectionalLightComponent caster;

    auto registry = scene->GetRegistry();
    auto view = registry->view<DirectionalLightComponent>();
    for (auto [id, dir] : view.each()) {
        if (dir.CastShadows) {
            caster = dir;
            break;
        }
    }
    if (!caster.CastShadows)
        return;

    // Update
    UpdateCascades(frame, scene, caster);

    // Render
    Vector<::Ref<RenderPassResource>> cascades = {
        RendererTools::Get("ShadowCascade0"),
        RendererTools::Get("ShadowCascade1"),
        RendererTools::Get("ShadowCascade2"),
        RendererTools::Get("ShadowCascade3")
    };

    // Update frustum buffer
    {
        auto ringBuffer = RendererTools::Get("CascadeBuffer");
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            mCascades[i].SRVIndex = cascades[i]->Descriptor(ViewType::ShaderResource);
        }

        ringBuffer->RBuffer[frame.FrameIndex]->CopyMapped(mCascades.data(), sizeof(Cascade) * SHADOW_CASCADE_COUNT);
    }

    frame.CommandBuffer->BeginMarker("Cascaded Shadow Maps");
    frame.CommandBuffer->SetMeshPipeline(mCascadePipeline);
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        frame.CommandBuffer->BeginMarker("Cascade " + std::to_string(i));
        frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::DepthWrite);
        frame.CommandBuffer->SetRenderTargets({}, cascades[i]->GetView(ViewType::DepthTarget));
        frame.CommandBuffer->ClearDepth(cascades[i]->GetView(ViewType::DepthTarget));
        frame.CommandBuffer->SetViewport(0, 0, DIR_LIGHT_SHADOW_DIMENSION, DIR_LIGHT_SHADOW_DIMENSION);
        frame.CommandBuffer->SetTopology(Topology::TriangleList);
        std::function<void(Frame frame, MeshNode*, Mesh* model, glm::mat4 transform)> drawNode = [&](Frame frame, MeshNode* node, Mesh* model, glm::mat4 transform) {
            if (!node) {
                return;
            }

            for (MeshPrimitive primitive : node->Primitives) {
                struct PushConstants {
                    int VertexBuffer;
                    int IndexBuffer;
                    int MeshletBuffer;
                    int MeshletVertices;
                    int MeshletTriangleBuffer;
                    glm::ivec3 Pad;

                    glm::mat4 Transform;
                    glm::mat4 View;
                    glm::mat4 Proj;
                } data = {
                    primitive.VertexBuffer->SRV(),
                    primitive.IndexBuffer->SRV(),
                    primitive.MeshletBuffer->SRV(),
                    primitive.MeshletVertices->SRV(),
                    primitive.MeshletTriangles->SRV(),
                    glm::ivec3(0),
                    
                    transform,
                    mCascades[i].View,
                    mCascades[i].Proj
                };
                frame.CommandBuffer->Barrier(primitive.VertexBuffer, ResourceLayout::Shader);
                frame.CommandBuffer->Barrier(primitive.IndexBuffer, ResourceLayout::Shader);
                frame.CommandBuffer->Barrier(primitive.MeshletBuffer, ResourceLayout::Shader);
                frame.CommandBuffer->Barrier(primitive.MeshletVertices, ResourceLayout::Shader);
                frame.CommandBuffer->Barrier(primitive.MeshletTriangles, ResourceLayout::Shader);
                frame.CommandBuffer->GraphicsPushConstants(&data, sizeof(data), 0);
                frame.CommandBuffer->DispatchMesh(primitive.MeshletCount, primitive.IndexCount / 3);
                frame.CommandBuffer->Barrier(primitive.VertexBuffer, ResourceLayout::Common);
                frame.CommandBuffer->Barrier(primitive.IndexBuffer, ResourceLayout::Common);
                frame.CommandBuffer->Barrier(primitive.MeshletBuffer, ResourceLayout::Common);
                frame.CommandBuffer->Barrier(primitive.MeshletVertices, ResourceLayout::Common);
                frame.CommandBuffer->Barrier(primitive.MeshletTriangles, ResourceLayout::Common);
            }
            if (!node->Children.empty()) {
                for (MeshNode* child : node->Children) {
                    drawNode(frame, child, model, transform);
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
                    drawNode(frame, mesh.MeshAsset->Mesh.Root, &mesh.MeshAsset->Mesh, entity.GetWorldTransform());
                }
            }
        }
        frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void Shadows::UpdateCascades(const Frame& frame, ::Ref<Scene> scene, DirectionalLightComponent caster)
{
    CameraComponent* camera = scene->GetMainCamera();
    if (!camera)
        return;
    if (!camera->Volume)
        return;

    UInt32 cascadeSize = DIR_LIGHT_SHADOW_DIMENSION;
    Vector<float> splits(SHADOW_CASCADE_COUNT + 1);

    // Precompute cascade splits using logarithmic split
    splits[0] = camera->Near;
    splits[SHADOW_CASCADE_COUNT] = camera->Far;
    for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
        float linearSplit = camera->Near + (camera->Far - camera->Far) * (static_cast<float>(i) / SHADOW_CASCADE_COUNT);
        float logSplit = camera->Near * std::pow(camera->Far / camera->Near, static_cast<float>(i) / SHADOW_CASCADE_COUNT);

        // Blend the splits using the lambda parameter
        splits[i] = camera->Volume->Volume.CascadeSplitLambda * logSplit + (1.0f - camera->Volume->Volume.CascadeSplitLambda) * linearSplit;
    }

    for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
        // Get frustum corners for the cascade in view space
        Vector<glm::vec4> corners = Math::CascadeCorners(camera->View, glm::radians(camera->FOV), (float)frame.Width / (float)frame.Height, splits[i], splits[i + 1]);

        // Calculate center
        glm::vec3 center(0.0f);
        for (const glm::vec4& corner : corners) {
            center += glm::vec3(corner);
        }
        center /= corners.size();

        // Adjust light's up vector
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(caster.Direction, up)) > 0.999f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        // Calculate light-space bounding sphere
        glm::vec3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
        float sphereRadius = 0.0f;
        for (auto& corner : corners) {
            float dist = glm::length(glm::vec3(corner) - center);
            sphereRadius = std::max(sphereRadius, dist);
        }
        sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
        maxBounds = glm::vec3(sphereRadius);
        minBounds = -maxBounds;

        // Get extents and create view matrix
        glm::vec3 cascadeExtents = maxBounds - minBounds;
        glm::vec3 shadowCameraPos = center - caster.Direction;

        glm::mat4 lightView = glm::lookAt(shadowCameraPos, center, up);
        glm::mat4 lightProjection = glm::ortho(
            minBounds.x,
            maxBounds.x,
            minBounds.y,
            maxBounds.y,
            minBounds.z,
            maxBounds.z
        );

        // Texel snap
        {
            glm::mat4 shadowMatrix = lightProjection * lightView;
            glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            shadowOrigin = shadowMatrix * shadowOrigin;
            shadowOrigin = glm::scale(glm::mat4(1.0f), glm::vec3(cascadeSize / 2)) * shadowOrigin;
    
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * (2.0f / cascadeSize);
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;
            lightProjection[3] += roundOffset;
        }

        // Store results
        mCascades[i].Split = splits[i + 1];
        mCascades[i].View = lightView;
        mCascades[i].Proj = lightProjection;
    }
}
