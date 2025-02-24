//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 11:59:14
//

#include "PhysicsSystem.hpp"

#include <Core/Logger.hpp>

PhysicsSystem::Data PhysicsSystem::sData;

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
        case PhysicsLayers::NON_MOVING:
            return "NON_MOVING";
        case PhysicsLayers::MOVING:
            return "MOVING";
        case PhysicsLayers::CHARACTER:
            return "CHARACTER";
        case PhysicsLayers::CHARACTER_GHOST:
            return "CHARACTER_GHOST";
        case PhysicsLayers::TRIGGER:
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
        mObjectToBroadPhase[PhysicsLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[PhysicsLayers::MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[PhysicsLayers::CHARACTER] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[PhysicsLayers::CHARACTER_GHOST] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[PhysicsLayers::TRIGGER] = BroadPhaseLayers::MOVING;
    }

    virtual JPH::uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        using namespace JPH;
        JPH_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }
private:
    JPH::BroadPhaseLayer					mObjectToBroadPhase[PhysicsLayers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case PhysicsLayers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case PhysicsLayers::MOVING:
            return true;
        case PhysicsLayers::TRIGGER:
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
        case PhysicsLayers::TRIGGER:
            return inObject2 == PhysicsLayers::CHARACTER_GHOST || inObject2 == PhysicsLayers::CHARACTER;
        case PhysicsLayers::NON_MOVING:
            return inObject2 == PhysicsLayers::MOVING || inObject2 == PhysicsLayers::CHARACTER_GHOST || inObject2 == PhysicsLayers::CHARACTER; // Non moving only collides with moving
        case PhysicsLayers::MOVING:
            return true; // Moving collides with everything
        case PhysicsLayers::CHARACTER_GHOST:
            return inObject2 != PhysicsLayers::CHARACTER;
        case PhysicsLayers::CHARACTER:
            return inObject2 != PhysicsLayers::CHARACTER_GHOST;
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

    const UInt32 maxBodies = 65536;
    const UInt32 numBodyMutexes = 0;
    const UInt32 maxBodyPairs = 65536 / 2;
    const UInt32 maxContactConstraints = 10240;

    sData.System = new JPH::PhysicsSystem;
    sData.System->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, JoltBroadphaseLayerInterface, JoltObjectVSBroadphaseLayerFilter, JoltObjectVSObjectLayerFilter);

    sData.ContactListener = new MyContactListener;
    sData.System->SetContactListener(sData.ContactListener);
    sData.System->SetGravity(JPH::Vec3(0.0f, -3.0f, 0.0f));

    sData.BodyInterface = &sData.System->GetBodyInterface();
    sData.JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);
    sData.DebugDraw = new PhysicsDebugDraw;

    JPH::DebugRenderer::sInstance = sData.DebugDraw;

    LOG_INFO("Initialized Physics System");
}

void PhysicsSystem::Exit()
{
    JPH::DebugRenderer::sInstance = nullptr;

    delete sData.DebugDraw;
    delete sData.JobSystem;
    delete sData.ContactListener;
    delete sData.System;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::Update(Ref<Scene> scene, float minStepDuration)
{
    int collisionSteps = 1;
    try {
        auto allocator = MakeRef<JPH::TempAllocatorMalloc>();

        auto error = sData.System->Update(minStepDuration, collisionSteps, allocator.get(), sData.JobSystem);
        if (error != JPH::EPhysicsUpdateError::None) {
            const char* errMessage = "";
            switch (error) {
                case JPH::EPhysicsUpdateError::ManifoldCacheFull:
                    errMessage = "Manifold cache full";
                    break;
                case JPH::EPhysicsUpdateError::BodyPairCacheFull:
                    errMessage = "Body pair cache full";
                    break;
                case JPH::EPhysicsUpdateError::ContactConstraintsFull:
                    errMessage = "contact constraints full";
                    break;
            }
            LOG_ERROR("Jolt Error: {0}", errMessage);
        }
    } catch (...) {
        LOG_CRITICAL("Jolt failed to update physics!");
    }
}
