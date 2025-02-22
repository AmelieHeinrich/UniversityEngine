//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 18:39:37
//

#include "PhysicsDebugDraw.hpp"

#include <Renderer/Passes/Debug.hpp>

void PhysicsDebugDraw::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    Debug::DrawLine(glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
                    glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
                    glm::vec3(inColor.r / 255.0f, inColor.g / 255.0f, inColor.b / 255.0f));
}
