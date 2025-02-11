//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 22:48:12
//

#include "Scene.hpp"

Scene::Scene()
{
    mEntities.reserve(64);
}

Scene::~Scene()
{
    for (auto& entity : mEntities) {
        if (entity->HasComponent<MeshComponent>()) {
            entity->GetComponent<MeshComponent>().Free();
        }
        mRegistry.destroy(entity->ID);
        delete entity;
    }
    mEntities.clear();
}

void Scene::Update()
{
    // Transform update
    {
        for (auto [entity, transform] : mRegistry.view<TransformComponent>().each()) {
            transform.Update();
        }
    }

    // Camera Update
    {
        auto view = mRegistry.view<TransformComponent, CameraComponent>();
        for (auto [entity, transform, camera] : view.each()) {
            camera.Update(transform.Position, transform.Rotation);
        }
    }
}

SceneCamera Scene::GetMainCamera()
{
    // NOTE(amelie): This is professional grade spaghetti bullshit but lowkey iterating through entities is fast as hell. Love EnTT x

    Vector<CameraComponent> cameras;
    auto view = mRegistry.view<CameraComponent>();
    for (auto [entity, camera] : view.each()) {
        cameras.push_back(camera);
    }

    std::sort(cameras.begin(), cameras.end(), [](const CameraComponent& a, const CameraComponent& b) {
        return a.Primary > b.Primary;
    });

    if (!cameras.empty()) {
        const auto& bestCamera = cameras.front();
        if (bestCamera.Primary > 0)
            return { bestCamera.View, bestCamera.Projection };
    }
    return { glm::mat4(0.0f), glm::mat4(0.0f) };
}

Entity* Scene::AddEntity(const String& name)
{
    Entity* newEntity = new Entity(&mRegistry);
    newEntity->ID = mRegistry.create();
    newEntity->Name = name;
    
    newEntity->AddComponent<TransformComponent>();
    newEntity->AddComponent<ScriptComponent>();

    mEntities.push_back(newEntity);
    return newEntity;
}

void Scene::RemoveEntity(Entity* e)
{
    if (e->HasComponent<MeshComponent>()) {
        e->GetComponent<MeshComponent>().Free();
    }
    mRegistry.destroy(e->ID);
    for (int i = 0; i < mEntities.size(); i++) {
        if (mEntities[i]->ID == e->ID)
            mEntities.erase(mEntities.begin() + i);
    }
}
