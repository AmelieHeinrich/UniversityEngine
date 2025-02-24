//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-24 12:47:34
//

#include "Rigidbody.hpp"

#include "PhysicsSystem.hpp"

Rigidbody::Rigidbody()
{
    mBody = nullptr;
}

Rigidbody::Rigidbody(PhysicsShape& shape, float mass, bool isStatic)
{
    Create(shape, mass, isStatic);
}

Rigidbody::~Rigidbody()
{
    PhysicsSystem::GetInterface()->DestroyBody(mBody->GetID());
}

void Rigidbody::Create(PhysicsShape& shape, float mass, bool isStatic)
{
    JPH::BodyCreationSettings settings(reinterpret_cast<const JPH::Shape*>(shape.GetShape().GetPtr()),
                                       JPH::Vec3::sZero(),
                                       JPH::Quat::sIdentity(),
                                       isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
                                       PhysicsLayers::MOVING);

    settings.mMassPropertiesOverride.mMass = mass;
    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;

    mBody = PhysicsSystem::GetInterface()->CreateBody(settings);
}
