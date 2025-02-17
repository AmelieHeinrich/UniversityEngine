//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 17:50:32
//

#pragma once

#include <Renderer/RenderPass.hpp>
#include <Renderer/Skybox.hpp>

class SkyboxForward : public RenderPass
{
public:
    SkyboxForward(RHI::Ref rhi);
    ~SkyboxForward() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
private:
    ::Ref<Skybox> mTestSkybox;
    Buffer::Ref mCubeBuffer;
    GraphicsPipeline::Ref mPipeline;
};
