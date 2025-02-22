//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 11:59:14
//

#include "PhysicsSystem.hpp"

#include <Core/Logger.hpp>

PhysicsSystem::Data PhysicsSystem::sData;

namespace Layers
{
    static constexpr UInt8 NON_MOVING = 0;
    static constexpr UInt8 MOVING = 1;
    static constexpr UInt8 CHARACTER = 2;
    static constexpr UInt8 CHARACTER_GHOST = 3;
    static constexpr UInt8 TRIGGER = 4;
    static constexpr UInt8 NUM_LAYERS = 5;
};

namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr Uint32 NUM_LAYERS(2);
};

String LayerToString(UInt8 layer)
{
    switch (layer)
    {
        case Layers::NON_MOVING:
            return "NON_MOVING";
        case Layers::MOVING:
            return "MOVING";
        case Layers::CHARACTER:
            return "CHARACTER";
        case Layers::CHARACTER_GHOST:
            return "CHARACTER_GHOST";
        case Layers::TRIGGER:
            return "TRIGGER";
    }
    return "SKIBIDI";
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::CHARACTER] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::CHARACTER_GHOST] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[Layers::TRIGGER] = BroadPhaseLayers::MOVING;
    }

    virtual JPH::uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        using namespace JPH;
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }
private:
    JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::MOVING:
            return true;
        case Layers::TRIGGER:
            return inLayer2 == BroadPhaseLayers::MOVING;
        default:
            return false;
        }
    }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::TRIGGER:
            return inObject2 == Layers::CHARACTER_GHOST || inObject2 == Layers::CHARACTER;
        case Layers::NON_MOVING:
            return inObject2 == Layers::MOVING || inObject2 == Layers::CHARACTER_GHOST || inObject2 == Layers::CHARACTER; // Non moving only collides with moving
        case Layers::MOVING:
            return true; // Moving collides with everything
        case Layers::CHARACTER_GHOST:
            return inObject2 != Layers::CHARACTER;
        case Layers::CHARACTER:
            return inObject2 != Layers::CHARACTER_GHOST;
        default:
            return false;
        }
    }
};

class MyContactListener : public JPH::ContactListener
{
public:
    virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
    {
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
    }

    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
    {
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
    {
    }
};

BPLayerInterfaceImpl JoltBroadphaseLayerInterface = BPLayerInterfaceImpl();
ObjectVsBroadPhaseLayerFilterImpl JoltObjectVSBroadphaseLayerFilter = ObjectVsBroadPhaseLayerFilterImpl();
ObjectLayerPairFilterImpl JoltObjectVSObjectLayerFilter;

void PhysicsSystem::Init()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    const UInt32 maxBodies = 4096;
    const UInt32 numBodyMutexes = 0;
    const UInt32 maxBodyPairs = 2048;
    const UInt32 maxContactConstraints = 2048;

    sData.System = new JPH::PhysicsSystem;
    sData.System->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, JoltBroadphaseLayerInterface, JoltObjectVSBroadphaseLayerFilter, JoltObjectVSObjectLayerFilter);

    sData.ContactListener = new MyContactListener;
    sData.System->SetContactListener(sData.ContactListener);
    sData.System->SetGravity(JPH::Vec3(0.0f, -3.0f, 0.0f));

    sData.BodyInterface = &sData.System->GetBodyInterface();
    sData.JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);

    LOG_INFO("Initialized Physics System");
}

void PhysicsSystem::Exit()
{
    delete sData.JobSystem;
    delete sData.ContactListener;
    delete sData.System;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::Update(Ref<Scene> scene, float minStepDuration)
{
    
}
