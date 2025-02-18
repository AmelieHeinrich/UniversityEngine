//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-18 09:56:06
//

#include "Shadows.hpp"

#include <Utility/Math.hpp>

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
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3 + sizeof(int) * 8);

    mCascadePipeline = rhi->CreateMeshPipeline(specs);
}

void Shadows::Render(const Frame& frame, ::Ref<Scene> scene)
{
    
}

void Shadows::UpdateCascades(const Frame& frame, ::Ref<Scene> scene)
{
    CameraComponent* camera = scene->GetMainCamera();
    
    DirectionalLightComponent light = {};
    for (auto [id, dir] : scene->GetRegistry()->view<DirectionalLightComponent>().each()) {
        if (dir.CastShadows) {
            light = dir;
            break;
        }
    }
    if (!light.CastShadows) {
        return;
    }

    UInt32 cascadeSize = DIR_LIGHT_SHADOW_DIMENSION;
    Vector<float> splits(SHADOW_CASCADE_COUNT + 1);

    // Precompute cascade splits using logarithmic split
    splits[0] = camera->Near;
    splits[SHADOW_CASCADE_COUNT] = camera->Far;
    for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
        float linearSplit = camera->Near + (camera->Far - camera->Far) * (static_cast<float>(i) / SHADOW_CASCADE_COUNT);
        float logSplit = camera->Near * std::pow(camera->Far / camera->Near, static_cast<float>(i) / SHADOW_CASCADE_COUNT);

        // Blend the splits using the lambda parameter
        splits[i] = mShadowSplitLambda * logSplit + (1.0f - mShadowSplitLambda) * linearSplit;
    }

    for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
        // Get frustum corners for the cascade in view space
        Vector<glm::vec4> corners = Math::CascadeCorners(camera->View, camera->FOV, (float)frame.Width / (float)frame.Height, splits[i], splits[i + 1]);

        // Calculate center
        glm::vec3 center(0.0f);
        for (const glm::vec4& corner : corners) {
            center += glm::vec3(corner);
        }
        center /= corners.size();

        // Adjust light's up vector
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(light.Direction, up)) > 0.999f) {
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
        glm::vec3 shadowCameraPos = center - light.Direction;

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
