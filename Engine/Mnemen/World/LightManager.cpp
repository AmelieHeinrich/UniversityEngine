//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 22:37:38
//

#include "LightManager.hpp"

#include <Renderer/RendererTools.hpp>
#include <Core/Profiler.hpp>
#include <Utility/Math.hpp>

LightManager::Data LightManager::sData;

void LightManager::Init(RHI::Ref rhi)
{
    RendererTools::CreateSharedRingBuffer("LightBuffer", 256);
    auto dir = RendererTools::CreateSharedRingBuffer("DirLightBuffer", 16384, sizeof(DirectionalLightComponent));
    auto point = RendererTools::CreateSharedRingBuffer("PointLightBuffer", 16384, sizeof(PointLightComponent));
    auto spot = RendererTools::CreateSharedRingBuffer("SpotLightBuffer", 16384, sizeof(SpotLightComponent));

    sData.Data.DirLightSRV = dir->Descriptor(ViewType::ShaderResource);
    sData.Data.PointLightSRV = point->Descriptor(ViewType::ShaderResource);
    sData.Data.SpotLightSRV = spot->Descriptor(ViewType::ShaderResource);
}

void LightManager::Update(const Frame& frame, Ref<Scene> scene)
{
    PROFILE_FUNCTION();

    auto registry = scene->GetRegistry();
    auto light = RendererTools::Get("LightBuffer");
    auto dir = RendererTools::Get("DirLightBuffer");
    auto point = RendererTools::Get("PointLightBuffer");
    auto spot = RendererTools::Get("SpotLightBuffer");

    sData.Data.DirLightCount = 0;
    sData.Data.PointLightCount = 0;
    sData.Data.SpotLightCount = 0;
    sData.Data.DirLightSRV = dir->Descriptor(ViewType::ShaderResource, frame.FrameIndex);
    sData.Data.PointLightSRV = point->Descriptor(ViewType::ShaderResource, frame.FrameIndex);
    sData.Data.SpotLightSRV = spot->Descriptor(ViewType::ShaderResource, frame.FrameIndex);

    {
        auto view = registry->view<TransformComponent, DirectionalLightComponent>();
        for (auto [id, t, dir] : view.each()) {
            dir.Direction = Math::QuatToForward(t.Rotation);
            sData.DirLights[sData.Data.DirLightCount] = dir;
            sData.Data.DirLightCount++;
        }
        dir->RBuffer[frame.FrameIndex]->CopyMapped(sData.DirLights.data(), sizeof(DirectionalLightComponent) * sData.Data.DirLightCount);
    }
    {
        auto view = registry->view<TransformComponent, PointLightComponent>();
        for (auto [id, t, dir] : view.each()) {
            dir.Position = t.Position;
            sData.PointLights[sData.Data.PointLightCount] = dir;
            sData.Data.PointLightCount++;
        }
        point->RBuffer[frame.FrameIndex]->CopyMapped(sData.PointLights.data(), sizeof(PointLightComponent) * sData.Data.PointLightCount);
    }
    {
        auto view = registry->view<TransformComponent, SpotLightComponent>();
        for (auto [id, t, dir] : view.each()) {
            dir.Direction = Math::QuatToForward(t.Rotation);
            sData.SpotLights[sData.Data.SpotLightCount] = dir;
            sData.Data.SpotLightCount++;
        }
        spot->RBuffer[frame.FrameIndex]->CopyMapped(sData.SpotLights.data(), sizeof(SpotLightComponent) * sData.Data.SpotLightCount);
    }
    light->RBuffer[frame.FrameIndex]->CopyMapped(&sData.Data, sizeof(sData.Data));
}
