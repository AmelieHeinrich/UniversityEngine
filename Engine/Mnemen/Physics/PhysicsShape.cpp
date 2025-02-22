//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 16:54:11
//

#include "PhysicsShape.hpp"

#include <Core/Application.hpp>
#include <fstream>

void PhysicsShape::Create(JPH::Ref<JPH::Shape> shape)
{
    mShape = JPH::Ref<JPH::ScaledShape>(new JPH::ScaledShape(shape, JPH::Vec3::sReplicate(1.0f)));
}

void PhysicsShape::Load(const String& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        LOG_ERROR("Cannot read physics shape {0}", path);
        return;
    }

    JPH::StreamInWrapper streamIn(stream);
    JPH::ScaledShape::ShapeResult result = JPH::ScaledShape::sRestoreFromBinaryState(streamIn);
    if (result.HasError()) {
        LOG_ERROR("Jolt: Error when reading the physics shape {0}", result.GetError());
        return;
    }

    mShape = JPH::Ref<JPH::ScaledShape>(new JPH::ScaledShape(result.Get(), JPH::Vec3::sReplicate(1.0f)));
}

void PhysicsShape::Save(const String& path)
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        LOG_ERROR("Cannot write physics shape to disk!");
        return;
    }

    JPH::StreamOutWrapper streamOut(stream);
    mShape->SaveBinaryState(streamOut);
    stream.close();
}

void PhysicsShape::SetScale(glm::vec3 scale)
{
    JPH::Vec3 joltScale = JPH::Vec3(scale.x, scale.y, scale.z);
    mShape->ScaleShape(joltScale);
}

glm::vec3 PhysicsShape::GetScale()
{
    auto scale = mShape->GetScale();
    return glm::vec3(scale.GetX(), scale.GetY(), scale.GetZ());
}
