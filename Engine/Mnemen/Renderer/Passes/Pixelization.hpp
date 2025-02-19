//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 18:59:01
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Pixelization : public RenderPass
{
public:
    Pixelization(RHI::Ref rhi);
    ~Pixelization() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
private:
    ComputePipeline::Ref mPipeline;
};
