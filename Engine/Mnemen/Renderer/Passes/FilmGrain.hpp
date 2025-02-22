//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 18:59:01
//

#pragma once

#include <Renderer/RenderPass.hpp>
#include <Core/Timer.hpp>

class FilmGrain : public RenderPass
{
public:
    FilmGrain(RHI::Ref rhi);
    ~FilmGrain() = default;

    void Render(const Frame& frame, ::Ref<Scene> scene) override;
private:
    ComputePipeline::Ref mPipeline;
    Timer mTimer; 
};
