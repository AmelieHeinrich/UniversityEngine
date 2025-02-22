//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 17:11:30
//

#include "Colliders.hpp"

#include <Core/Logger.hpp>

BoxCollider::BoxCollider(glm::vec3 halfExtent)
{
    JPH::BoxShapeSettings settings(JPH::Vec3(halfExtent.x, halfExtent.y, halfExtent.z), 0.05f);
    JPH::ShapeSettings::ShapeResult result = settings.Create();
    if (result.HasError()) {
        LOG_ERROR("Failed to create box shape!");
        return;
    }
    Create(result.Get());
}
