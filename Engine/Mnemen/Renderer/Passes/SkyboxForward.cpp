//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 17:52:46
//

#include "SkyboxForward.hpp"

#include <RHI/Uploader.hpp>
#include <Renderer/SkyboxCooker.hpp>
#include <Core/Profiler.hpp>

const float CubeVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

SkyboxForward::SkyboxForward(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Skybox/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Skybox/Fragment.hlsl", AssetType::Shader);

    GraphicsPipelineSpecs specs;
    specs.Fill = FillMode::Solid;
    specs.Cull = CullMode::None;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::LEqual;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Formats.push_back(TextureFormat::RGBA16Float);
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) + sizeof(glm::mat4));
    specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;

    mPipeline = rhi->CreateGraphicsPipeline(specs);

    mCubeBuffer = rhi->CreateBuffer(sizeof(CubeVertices), sizeof(float) * 3, BufferType::Vertex, "Skybox Vertex Buffer");
    Uploader::EnqueueBufferUpload((void*)CubeVertices, sizeof(CubeVertices), mCubeBuffer);
}

void SkyboxForward::Render(const Frame& frame, ::Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    CameraComponent* mainCamera = scene->GetMainCamera();
    if (!mainCamera)
        return;
    if (!mainCamera->Volume)
        return;
    if (!mainCamera->Volume->Volume.EnableSkybox)
        return;

    glm::mat4 mvp = mainCamera->Projection * glm::mat4(glm::mat3(mainCamera->View)) * glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f));
    ::Ref<Skybox> skybox = scene->GetSkybox();

    auto sampler = RendererTools::Get("MaterialSampler");
    auto depthBuffer = RendererTools::Get("GBufferDepth");
    auto colorBuffer = RendererTools::Get("HDRColorBuffer");
    
    struct Data {
        int env;
        int sampler;
        glm::vec2 pad;

        glm::mat4 mvp;
    } data = {
        skybox->EnvironmentMapSRV->GetDescriptor().Index,
        sampler->Sampler->BindlesssSampler(),
        glm::vec2(0.0),
        mvp
    };

    frame.CommandBuffer->BeginMarker("Skybox");
    frame.CommandBuffer->SetViewport(0, 0, frame.Width, frame.Height);
    frame.CommandBuffer->SetTopology(Topology::TriangleList);
    frame.CommandBuffer->Barrier(colorBuffer->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(depthBuffer->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->SetRenderTargets({ colorBuffer->GetView(ViewType::RenderTarget) }, depthBuffer->GetView(ViewType::DepthTarget));
    frame.CommandBuffer->GraphicsPushConstants(&data, sizeof(data), 0);
    frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
    frame.CommandBuffer->SetVertexBuffer(mCubeBuffer);
    frame.CommandBuffer->Draw(36);
    frame.CommandBuffer->Barrier(depthBuffer->Texture, ResourceLayout::Common);
    frame.CommandBuffer->Barrier(colorBuffer->Texture, ResourceLayout::Shader);
    frame.CommandBuffer->EndMarker();
}
