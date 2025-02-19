//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:00:29
//

#include "FilmGrain.hpp"

FilmGrain::FilmGrain(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // Code goes here
}

void FilmGrain::Render(const Frame& frame, ::Ref<Scene> scene)
{
    CameraComponent* camera = scene->GetMainCamera();
    if (!camera)
        return;
    if (!camera->Volume)
        return;

    // Your settings are in camera->Volume->Volume
    //

    frame.CommandBuffer->BeginMarker("Posterization");
    // Code goes here
    frame.CommandBuffer->EndMarker();
}
