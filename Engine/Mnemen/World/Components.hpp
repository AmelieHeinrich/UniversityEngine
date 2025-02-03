//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 00:01:27
//

#pragma once

#include <Asset/AssetManager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent
{
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Scale = glm::vec3(1.0f);
    glm::vec3 Rotation = glm::vec3(0.0f);
    glm::mat4 Matrix;

    void Update();
};

struct MeshComponent
{
    Asset::Handle MeshAsset;

    void Init(const String& string);
};

struct CameraComponent
{
    bool Primary = true;
    float FOV = 90.0f;
    float Near = 0.1f;
    float Far = 200.0f;
    float AspectRatio = 1.77777777778f;

    glm::mat4 View = glm::mat4(1.0f);
    glm::mat4 Projection = glm::mat4(1.0f);

    void Update(glm::vec3 Position, glm::vec3 Rotation);
};
