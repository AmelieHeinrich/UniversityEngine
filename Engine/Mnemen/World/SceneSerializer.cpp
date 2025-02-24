//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-06 14:16:34
//

#include "SceneSerializer.hpp"

#include <Renderer/SkyboxCooker.hpp>
#include <Core/File.hpp>
#include <Core/Logger.hpp>

#include <Utility/Math.hpp>

void SceneSerializer::SerializeScene(Ref<Scene> scene, const String& path)
{
    auto registry = scene->GetRegistry();

    nlohmann::json root;
    root["entities"] = nlohmann::json::array();
    root["skybox"] = scene->GetSkybox()->Path;
    
    registry->view<entt::entity>().each([&](entt::entity id) {
        Entity entity(registry);
        entity.ID = id;

        if (entity.HasComponent<PrivateComponent>())
            return;

        auto entityJson = SerializeEntity(entity);
        root["entities"].push_back(entityJson);
    });

    File::WriteJSON(root, path);
    LOG_INFO("Saved scene at {0}", path);
}

Ref<Scene> SceneSerializer::DeserializeScene(const String& path)
{
    Ref<Scene> scene = MakeRef<Scene>();

    nlohmann::json root = File::LoadJSON(path);
    UnorderedMap<UInt32, Entity> entityMap;

    scene->GetSkybox()->Path = root["skybox"];

    for (const auto& entityJson : root["entities"]) {
        DeserializeEntity(scene, entityJson, entityMap);
    }

    for (const auto& entityJson : root["entities"]) {
        Entity entity = entityMap[entityJson["id"].get<UInt32>()];
        if (!entityJson["parent"].is_null()) {
            Entity parent = entityMap[entityJson["parent"].get<UInt32>()];
            entity.SetParent(parent);
        }
    }

    LOG_INFO("Loaded scene at {0}", path);
    return scene;
}

nlohmann::json SceneSerializer::SerializeEntity(Entity entity)
{
    nlohmann::json entityJson;
    entityJson["id"] = static_cast<UInt32>(entity.ID);
    entityJson["name"] = entity.GetComponent<TagComponent>().Tag;

    if (entity.HasParent()) {
        entityJson["parent"] = static_cast<UInt32>(entity.GetParent().ID);
    } else {
        entityJson["parent"] = nullptr;
    }

    if (entity.HasComponent<TransformComponent>()) {
        glm::mat4 local = entity.GetWorldTransform();
        glm::vec3 p, r, s;
        Math::DecomposeTransform(local, p, r, s);
        glm::quat q = Math::EulerToQuat(r);

        entityJson["transform"] = {
            {"position", {p.x, p.y, p.z}},
            {"rotation", {q.x, q.y, q.z, q.w}},
            {"scale", {s.x, s.y, s.z}}
        };
    }

    if (entity.HasComponent<MeshComponent>()) {
        auto& mesh = entity.GetComponent<MeshComponent>();
        entityJson["mesh"] = {
            { "path", mesh.MeshAsset ? mesh.MeshAsset->Path : "" }
        };
    }

    if (entity.HasComponent<MaterialComponent>()) {
        auto& material = entity.GetComponent<MaterialComponent>();
        entityJson["material"] = {
            { "inherit", material.InheritFromModel },
            { "albedo", material.Albedo ? material.Albedo->Path : "" },
            { "normal", material.Normal ? material.Normal->Path : "" },
            { "pbr", material.PBR ? material.PBR->Path : "" }
        };
    }
    
    // Camera Component
    if (entity.HasComponent<CameraComponent>()) {
        CameraComponent camera = entity.GetComponent<CameraComponent>();
        entityJson["camera"] = {
            { "primary", (bool)camera.Primary },
            { "fov", camera.FOV },
            { "near", camera.Near },
            { "far", camera.Far },
            { "volumePath", camera.Volume->Path }
        };
    }

    // Script Component
    ScriptComponent scripts = entity.GetComponent<ScriptComponent>();
    entityJson["scripts"] = nlohmann::json::array();
    for (auto& instance : scripts.Instances) {
        entityJson["scripts"].push_back(instance->Handle->Path);
    }

    // Audio Source component
    if (entity.HasComponent<AudioSourceComponent>()) {
        AudioSourceComponent source = entity.GetComponent<AudioSourceComponent>();
        entityJson["audioSource"] = {
            { "volume", source.Volume },
            { "looping", source.Looping },
            { "playOnAwake", source.PlayOnAwake },
            { "path", source.Handle ? source.Handle->Path : nullptr }
        };
    }
    
    // Directional Light component
    if (entity.HasComponent<DirectionalLightComponent>()) {
        DirectionalLightComponent& dir = entity.GetComponent<DirectionalLightComponent>();
        entityJson["directionalLight"] = {
            { "strength", dir.Strength },
            { "color", { dir.Color.x, dir.Color.y, dir.Color.z } },
            { "castShadows", dir.CastShadows }
        };
    }

    // Point Light component
    if (entity.HasComponent<PointLightComponent>()) {
        PointLightComponent& dir = entity.GetComponent<PointLightComponent>();
        entityJson["pointLight"] = {
            { "radius", dir.Radius },
            { "color", { dir.Color.x, dir.Color.y, dir.Color.z } },
        };
    }

    // Spot Light component
    if (entity.HasComponent<SpotLightComponent>()) {
        SpotLightComponent& dir = entity.GetComponent<SpotLightComponent>();
        entityJson["spotLight"] = {
            { "radius", dir.Radius },
            { "outerRadius", dir.OuterRadius },
            { "castShadows", dir.CastShadows },
            { "color", { dir.Color.x, dir.Color.y, dir.Color.z } },
            { "strength", dir.Strength }
        };
    }

    // Box collider component
    if (entity.HasComponent<BoxCollider>()) {
        PhysicsShape& shape = entity.GetComponent<BoxCollider>();
        glm::vec3 scale = shape.GetScale();

        entityJson["box"] = {};
        entityJson["box"]["scale"][0] = scale.x;
        entityJson["box"]["scale"][1] = scale.y;
        entityJson["box"]["scale"][2] = scale.z;
    }

    // Sphere collider component
    if (entity.HasComponent<SphereCollider>()) {
        PhysicsShape& shape = entity.GetComponent<SphereCollider>();
        glm::vec3 scale = shape.GetScale();

        entityJson["sphere"] = {};
        entityJson["sphere"]["scale"][0] = scale.x;
        entityJson["sphere"]["scale"][1] = scale.y;
        entityJson["sphere"]["scale"][2] = scale.z;
    }

    // Capsule collider component
    if (entity.HasComponent<CapsuleCollider>()) {
        PhysicsShape& shape = entity.GetComponent<CapsuleCollider>();
        glm::vec3 scale = shape.GetScale();

        entityJson["capsule"] = {};
        entityJson["capsule"]["scale"][0] = scale.x;
        entityJson["capsule"]["scale"][1] = scale.y;
        entityJson["capsule"]["scale"][2] = scale.z;
    }

    // Convex hull collider component
    if (entity.HasComponent<ConvexHullCollider>()) {
        PhysicsShape& shape = entity.GetComponent<ConvexHullCollider>();
        glm::vec3 scale = shape.GetScale();

        entityJson["convex"] = {};
        entityJson["convex"]["scale"][0] = scale.x;
        entityJson["convex"]["scale"][1] = scale.y;
        entityJson["convex"]["scale"][2] = scale.z;
    }

    // Rigidbody component
    if (entity.HasComponent<Rigidbody>()) {
        entityJson["rigidbody"] = {};
    }

    return entityJson;
}

Entity SceneSerializer::DeserializeEntity(Ref<Scene> scene, const nlohmann::json& entityJson, UnorderedMap<UInt32, Entity>& entityMap)
{
    Entity entity = scene->AddEntity(entityJson["name"]);
    entityMap[entityJson["id"].get<UInt32>()] = entity;

    if (entityJson.contains("transform")) {
        auto& transform = entity.GetComponent<TransformComponent>();
        auto t = entityJson["transform"];
        transform.Position = {t["position"][0], t["position"][1], t["position"][2]};
        transform.Rotation = glm::quat(t["rotation"][3], t["rotation"][0], t["rotation"][1], t["rotation"][2]);
        transform.Scale = {t["scale"][0], t["scale"][1], t["scale"][2]};
        transform.Update();
    }
    if (entityJson.contains("mesh")) {
        auto& mesh = entity.AddComponent<MeshComponent>();
        auto m = entityJson["mesh"];
        mesh.Init(m["path"]);
    }
    if (entityJson.contains("camera")) {
        auto& camera = entity.AddComponent<CameraComponent>();
        auto c = entityJson["camera"];
        camera.Primary = c["primary"];
        camera.FOV = c["fov"];
        camera.Near = c["near"];
        camera.Far = c["far"];
        if (!c["volumePath"].get<String>().empty())
            camera.Volume = AssetManager::Get(c["volumePath"], AssetType::PostFXVolume);
    }
    if (entityJson.contains("audioSource")) {
        auto& audio = entity.AddComponent<AudioSourceComponent>();
        auto a = entityJson["audioSource"];
        if (!a["path"].is_null()) {
            audio.Init(a["path"]);
        }
        audio.Looping = a["looping"];
        audio.PlayOnAwake = a["playOnAwake"];
        audio.Volume = a["volume"];
    }
    if (entityJson.contains("material")) {
        auto& material = entity.AddComponent<MaterialComponent>();
        auto m = entityJson["material"];
        material.InheritFromModel = m["inherit"];
        material.LoadAlbedo(m["albedo"]);
        material.LoadNormal(m["normal"]);
        material.LoadPBR(m["pbr"]);
    }
    for (auto& script : entityJson["scripts"]) {
        auto& sc = entity.GetComponent<ScriptComponent>();
        sc.PushScript(script);
    }
    if (entityJson.contains("directionalLight")) {
        auto& dir = entity.AddComponent<DirectionalLightComponent>();
        auto d = entityJson["directionalLight"];
        dir.Strength = d["strength"];
        dir.CastShadows = d["castShadows"];
        dir.Color = glm::vec3(d["color"][0], d["color"][1], d["color"][2]);
    }
    if (entityJson.contains("pointLight")) {
        auto& dir = entity.AddComponent<PointLightComponent>();
        auto d = entityJson["pointLight"];
        dir.Radius = d["radius"];
        dir.Color = glm::vec3(d["color"][0], d["color"][1], d["color"][2]);
    }
    if (entityJson.contains("spotLight")) {
        auto& dir = entity.AddComponent<SpotLightComponent>();
        auto d = entityJson["spotLight"];
        dir.Radius = d["radius"];
        dir.OuterRadius = d["outerRadius"];
        dir.CastShadows = d["castShadows"];
        dir.Color = glm::vec3(d["color"][0], d["color"][1], d["color"][2]);
        dir.Strength = d["strength"];
    }
    if (entityJson.contains("box")) {
        auto& box = entity.AddComponent<BoxCollider>(glm::vec3(1.0f));
        auto b = entityJson["box"];
        box.SetScale(glm::vec3(b["scale"][0], b["scale"][1], b["scale"][2]));
    }
    if (entityJson.contains("sphere")) {
        auto& sphere = entity.AddComponent<SphereCollider>(1.0f);
        auto b = entityJson["sphere"];
        sphere.SetScale(glm::vec3(b["scale"][0], b["scale"][1], b["scale"][2]));
    }
    if (entityJson.contains("capsule")) {
        auto& capsule = entity.AddComponent<CapsuleCollider>(1.0f, 0.5f);
        auto b = entityJson["capsule"];
        capsule.SetScale(glm::vec3(b["scale"][0], b["scale"][1], b["scale"][2]));
    }
    if (entityJson.contains("convex")) {
        // TODO
    }
    if (entityJson.contains("rigidbody")) {
        auto& rb = entity.AddComponent<Rigidbody>();

        if (entity.HasComponent<BoxCollider>()) rb.Create(entity.GetComponent<BoxCollider>());
        if (entity.HasComponent<SphereCollider>()) rb.Create(entity.GetComponent<SphereCollider>());
        if (entity.HasComponent<CapsuleCollider>()) rb.Create(entity.GetComponent<CapsuleCollider>());
    }

    return entity;
}
