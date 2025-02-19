//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 18:59:01
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Posterization : public RenderPass
{
public:
    Posterization(RHI::Ref rhi);
    ~Posterization() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
private:
    ComputePipeline::Ref mPipeline;
};
