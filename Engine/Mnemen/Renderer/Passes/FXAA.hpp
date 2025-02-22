//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 14:12:59
//

#pragma once

#include <Renderer/RenderPass.hpp>

class FXAA : public RenderPass
{
public:
    FXAA(RHI::Ref rhi);
    ~FXAA() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
private:
    ComputePipeline::Ref mPipeline;
};
