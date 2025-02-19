//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-18 09:39:53
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int DIR_LIGHT_SHADOW_DIMENSION = 2048;
constexpr int SPOT_LIGHT_SHADOW_DIMENSION = 2048;
constexpr int SHADOW_CASCADE_COUNT = 4;

struct Cascade
{
    int SRVIndex;
    float Split;
    glm::ivec2 Pad;

    glm::mat4 View;
    glm::mat4 Proj;
};

struct SpotLightShadow
{
    int ParentID;
    SpotLightComponent* Parent;
    Texture::Ref ShadowMap;
    
    View::Ref SRV;
    View::Ref DSV;
};

class Shadows : public RenderPass
{
public:
    Shadows(RHI::Ref rhi);
    ~Shadows() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
    void RenderCascades(const Frame& frame, ::Ref<Scene> scene);
    void RenderSpot(const Frame& frame, ::Ref<Scene> scene);
private:
    void UpdateCascades(const Frame& frame, ::Ref<Scene> scene, DirectionalLightComponent caster);

    MeshPipeline::Ref mCascadePipeline = nullptr;
    Array<Cascade, SHADOW_CASCADE_COUNT> mCascades;
    Vector<SpotLightShadow> mSpotLightShadows;
    int once = 0;
};
