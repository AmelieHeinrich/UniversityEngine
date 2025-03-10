//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:00:29
//

#include "Pixelization.hpp"

Pixelization::Pixelization(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // Code goes here
}

void Pixelization::Render(const Frame& frame, ::Ref<Scene> scene)
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
