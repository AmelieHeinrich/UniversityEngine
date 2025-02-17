//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 22:44:35
//

#pragma once

#include "Entity.hpp"

#include <Renderer/Skybox.hpp>

/// @class Scene
/// @brief A representation of a scene.
///
/// The Scene class manages entities and a registry for a scene. It provides functionality 
/// to update the scene, retrieve the main camera, and manage entities within the scene.
class Scene
{
public:
    /// @brief Constructs a Scene object.
    Scene();
    
    /// @brief Destroys the Scene object.
    ~Scene();

    /// @brief Updates the scene.
    /// 
    /// This function is responsible for updating all entities and components within the scene.
    void Update();

    /// @brief Retrieves the main camera of the scene.
    /// 
    /// @return The main SceneCamera object.
    CameraComponent* GetMainCamera();

    /// @brief Retrieves the entity registry.
    /// 
    /// @return A pointer to the entity registry.
    entt::registry* GetRegistry() { return &mRegistry; }

    /// @brief Adds an entity to the scene.
    /// 
    /// @param name The name of the entity. Defaults to "Sigma Entity".
    /// @return A pointer to the newly created Entity object.
    Entity AddEntity(const String& name = "Sigma Entity");

    /// @brief Removes an entity from the scene.
    /// 
    /// @param e A pointer to the entity to be removed.
    void RemoveEntity(Entity e);

    /// @brief Sets the current skybox
    /// @param skybox The skybox to set
    void SetSkybox(Ref<Skybox> skybox) { mSkybox = skybox; }

    /// @brief Get the current loaded skybox
    /// @return A reference to the scene skybox
    Ref<Skybox> GetSkybox() { return mSkybox; }

    /// @brief Recooks the scene skybox from the given environment map path.
    void CookSkybox(const String& path);

    /// Default entities

    Entity AddDefaultCamera(const String& name = "Camera");
    Entity AddDefaultCube(const String& name = "Cube");
    Entity AddDefaultSphere(const String& name = "Sphere");
    Entity AddDefaultPlane(const String& name = "Plane");
    Entity AddDefaultCapsule(const String& name = "Capsule");
    Entity AddDefaultCylinder(const String& name = "Cylinder");

    ///
private:
    friend class Entity; ///< Allows Entity to access private members of Scene.
    friend class Renderer; ///< Allows Renderer to access private members of Scene.
    friend class PhysicsSystem; ///< Allows PhysicsSystem to access private members of Scene.
    friend class AISystem; ///< Allows AISystem to access private members of Scene.
    friend class AudioSystem; ///< Allows AudioSystem to access private members of Scene.
    friend class ScriptSystem; ///< Allows ScriptSystem to access private members of Scene.

    entt::registry mRegistry; ///< The registry that manages entities and components.
    Ref<Skybox> mSkybox;
};
