//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 11:59:14
//

#include "PhysicsSystem.hpp"

JPH::PhysicsSystem* sPhysicsSystem;
JPH::BodyInterface* sPhysicsWorld;
//JPH::Vec3 sGravity;
JPH::TempAllocatorMalloc* sAllocator;
constexpr JPH::BroadPhaseLayer sNonMoving(0) ;
constexpr JPH::BroadPhaseLayer sMoving(1);
constexpr int mLayers(2);
constexpr JPH::ObjectLayer sNonMovingLayer = 0;
constexpr JPH::ObjectLayer sMovingLayer = 1;
constexpr JPH::ObjectLayer sLayers = 2;


#include <Core/Logger.hpp>

static void TraceImpl(const char* inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
{
    // Print to the TTY
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

    // Breakpoint
    return true;
};

#endif // JPH_ENABLE_ASSERTS

bool MyObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer ObjectLayer1, JPH::ObjectLayer ObjectLayer2)
{
    switch (ObjectLayer1)
    {
    case sNonMovingLayer:
        return ObjectLayer2 == sMovingLayer;
    case sMovingLayer:
        return true;
    default:
        LOG_INFO("Initialized Collision pair System");
        return false;
    }
}

MyBroadPhaseLayerInterface::MyBroadPhaseLayerInterface()
{
    mObjectToBroadPhase[sNonMovingLayer] = sNonMoving;
    mObjectToBroadPhase[sMovingLayer] = sMoving;
}

unsigned int MyBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return mLayers;
}

JPH::BroadPhaseLayer MyBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer Layer) const{
    return mObjectToBroadPhase[sLayers];
}

bool MyObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer Layer1, JPH::BroadPhaseLayer Layer2) {
    switch (Layer1)
    {
    case sNonMovingLayer:
        return Layer2 == sMoving;
    case sMovingLayer:
        return true;
    default:
        LOG_INFO("Initialized Collision Broad Phase System");
        return false;
    }
}

void CreateShape::PlaneShape(JPH::BodyInterface scene)
{
    JPH::PlaneShapeSettings PlaneShapeSetting;
    PlaneShapeSetting.SetEmbedded();
    JPH::ShapeSettings::ShapeResult PlaneShapeResult = PlaneShapeSetting.Create();
    JPH::ShapeRefC PlaneShape = PlaneShapeResult.Get();
    JPH::BodyCreationSettings PlaneSetting(PlaneShape, JPH::RVec3(0.0f,-1.0f,0.0f),JPH::Quat::sIdentity(), JPH::EMotionType::Static,sNonMovingLayer);
    JPH::Body* Plane = scene.CreateBody(PlaneSetting);
    scene.AddBody(Plane->GetID(), JPH::EActivation::DontActivate);
}
//(JPH::Plane(JPH::Vec3(100.0f,1.0f,100.0f),1.0f), JPH::PhysicsMaterial, JPH::cDefaultHalfExtent)

void PhysicsSystem::Init()
{
    JPH::RegisterDefaultAllocator();

    JPH::Trace = TraceImpl;
    JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    int MaxBodies = 65536;
    int NumBodyMutexes = 0;
    int MaxBodyPairs = 65536;
    int MaxContactConstraints = 10240;
  
    MyBroadPhaseLayerInterface broad_phase_layer_interface ;
    MyObjectVsBroadPhaseLayerFilter object_vs_broadphase_layer_filter;
    MyObjectLayerPairFilter object_vs_object_layer_filter;

    //Create the allocator
    //sAllocator = new JPH::TempAllocatorImpl(1 * 1024 * 1024);
    sAllocator = new JPH::TempAllocatorMalloc;

    //Initialize the Jolt Physics system
    sPhysicsSystem = new JPH::PhysicsSystem();
    sPhysicsSystem->Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter );
    MyBodyActivationListener body_activation_listener;
    sPhysicsSystem->SetBodyActivationListener(&body_activation_listener);
    MyContactListener contact_listener;
    sPhysicsSystem->SetContactListener(&contact_listener);
    JPH::Vec3 sGravity = JPH::Vec3(0.0f, -9.81f, 0.0f);
    sPhysicsSystem->SetGravity(sGravity);

    sPhysicsSystem->OptimizeBroadPhase();


    LOG_INFO("Initialized Physics System");
}

void PhysicsSystem::Exit()
{
    // Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
    /*sPhysicsWorld->RemoveBody(sphere_id);

    // Destroy the sphere. After this the sphere ID is no longer valid.
    sPhysicsWorld->DestroyBody(sphere_id);

    // Remove and destroy the floor
    sPhysicsWorld->RemoveBody(floor->GetID());
    sPhysicsWorld->DestroyBody(floor->GetID());*/
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
    delete sPhysicsSystem;
    delete sAllocator;
}

void PhysicsSystem::Update(Ref<Scene> scene, float minStepDuration)
{
    JPH::JobSystemThreadPool job_system(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
    int cCollisionSteps = 1;
    // Ensure minStepDuration is within the acceptable range for the time step.
    if (minStepDuration > 0)
    {
        // Update the physics system with the given time step (minStepDuration).
        sPhysicsSystem->Update(minStepDuration, cCollisionSteps, sAllocator, &job_system);
    }
}
