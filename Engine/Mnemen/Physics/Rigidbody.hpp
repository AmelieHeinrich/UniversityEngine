//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-24 12:46:20
//

#pragma once

#include "Colliders.hpp"

class Rigidbody
{
public:
    Rigidbody();
    Rigidbody(PhysicsShape& shape, float mass = 1.0f, bool isStatic = false);
    ~Rigidbody();

    void Create(PhysicsShape& shape, float mass = 1.0f, bool isStatic = false);
private:
    JPH::Body* mBody;
};
