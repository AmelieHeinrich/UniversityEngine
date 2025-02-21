//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-19 19:00:29
//

#include "Posterization.hpp"

Posterization::Posterization(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // Code goes here
    // Haii!! It's me coralie!!
}

void Posterization::Render(const Frame& frame, ::Ref<Scene> scene)
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
