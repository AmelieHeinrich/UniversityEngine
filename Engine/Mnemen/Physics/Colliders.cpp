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
        LOG_ERROR("Failed to create box shape: {0}", result.GetError());
        return;
    }
    Create(result.Get());
}

SphereCollider::SphereCollider(float radius)
{
    JPH::SphereShapeSettings settings(radius);
    JPH::ShapeSettings::ShapeResult result = settings.Create();
    if (result.HasError()) {
        LOG_ERROR("Failed to create sphere shape: {0}", result.GetError());
        return;
    }
    Create(result.Get());
}

CapsuleCollider::CapsuleCollider(float radius, float height)
{
    JPH::CapsuleShapeSettings settings(height, radius);
    JPH::ShapeSettings::ShapeResult result = settings.Create();
    if (result.HasError()) {
        LOG_ERROR("Failed to create sphere shape: {0}", result.GetError());
        return;
    }
    Create(result.Get());
}

ConvexHullCollider::ConvexHullCollider(const JPH::Array<JPH::Vec3>& v)
{
    JPH::ConvexHullShapeSettings settings(v, 0.05f);
    JPH::ShapeSettings::ShapeResult result = settings.Create();
    if (result.HasError()) {
        LOG_ERROR("Failed to create convex hull shape: {0}", result.GetError());
        return;
    }
    Create(result.Get());
}
