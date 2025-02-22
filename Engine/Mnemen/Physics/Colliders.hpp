//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-22 17:05:32
//

#pragma once

#include "PhysicsShape.hpp"

class BoxCollider : public PhysicsShape
{
public:
    BoxCollider(glm::vec3 halfExtent);
};

class SphereCollider : public PhysicsShape
{
public:
    SphereCollider(float radius);
};

class CapsuleCollider : public PhysicsShape
{
public:
    CapsuleCollider(float radius, float height);
};

class ConvexHullCollider : public PhysicsShape
{
public:
    ConvexHullCollider(const JPH::Array<JPH::Vec3>& v);
};
