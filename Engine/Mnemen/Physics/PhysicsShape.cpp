//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 16:54:11
//

#include "PhysicsShape.hpp"

#include <Core/Application.hpp>
#include <fstream>

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
