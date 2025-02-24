//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 11:58:17
//

#pragma once

#include <World/Scene.hpp>

#include <iostream>
#include <cstdarg>
#include <thread>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ShapeCast.h>

#include "PhysicsDebugDraw.hpp"

class BPLayerInterfaceImpl;
class MyContactListener;

namespace PhysicsLayers
{
    static constexpr UInt8 NON_MOVING = 0;
    static constexpr UInt8 MOVING = 1;
    static constexpr UInt8 CHARACTER = 2;
    static constexpr UInt8 CHARACTER_GHOST = 3;
    static constexpr UInt8 TRIGGER = 4;
    static constexpr UInt8 NUM_LAYERS = 5;
};

class PhysicsSystem
{
public:
    static void Init();
    static void Exit();
    static void Update(Ref<Scene> scene, float minStepDuration);

    static JPH::BodyInterface* GetInterface() { return sData.BodyInterface; }
private:
    static struct Data {
        JPH::PhysicsSystem* System;
        JPH::JobSystemThreadPool* JobSystem;
        JPH::BodyInterface* BodyInterface;
        PhysicsDebugDraw* DebugDraw;

        MyContactListener* ContactListener;
        BPLayerInterfaceImpl* BroadphaseInterface;
    } sData;
};

