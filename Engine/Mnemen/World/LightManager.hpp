//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-17 22:33:02
//

#pragma once

#include <RHI/RHI.hpp>

#include "Scene.hpp"

struct LightData
{
    int DirLightSRV;
    int DirLightCount;
    int PointLightSRV;
    int PointLightCount;
    int SpotLightSRV;
    int SpotLightCount;
};

class LightManager
{
public:
    static void Init(RHI::Ref rhi);
    
    static void Update(const Frame& frame, Ref<Scene> scene);
private:
    static struct Data {
        LightData Data;

        Array<DirectionalLightComponent, 512> DirLights;
        Array<PointLightComponent, 1024> PointLights;
        Array<SpotLightComponent, 100> SpotLights;

        Array<Buffer::Ref, FRAMES_IN_FLIGHT> DirLightBuffer;
        Array<Buffer::Ref, FRAMES_IN_FLIGHT> PointLightBuffer;
        Array<Buffer::Ref, FRAMES_IN_FLIGHT> SpotLightBuffer;
        Array<Buffer::Ref, FRAMES_IN_FLIGHT> LightBuffer;
    } sData;
};
