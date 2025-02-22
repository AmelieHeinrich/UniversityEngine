//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 18:36:07
//

#pragma once

#include <jolt/Jolt.h>
#include <jolt/Renderer/DebugRenderer.h>
#include <jolt/Renderer/DebugRendererSimple.h>

#include <glm/glm.hpp>

class PhysicsDebugDraw : public JPH::DebugRendererSimple
{
public:
    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override {}
};
