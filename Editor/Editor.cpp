//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 12:26:03
//

#include "Editor.hpp"

#include <FontAwesome/FontAwesome.hpp>
#include <RHI/Uploader.hpp>

#include <imgui.h>
#include <imgui_internal.h>

Editor::Editor(ApplicationSpecs specs)
    : Application(specs)
{
    mCurrentScenePath = mProject->StartScenePathRelative;
    if (mCurrentScenePath.empty()) {
        NewScene();
    } else {
        mCameraEntity = mScene->AddEntity("Editor Camera");
        mCameraEntity.AddComponent<PrivateComponent>();
        auto& cam = mCameraEntity.AddComponent<CameraComponent>(true);
        cam.Primary = 2;
    }
    mScenePlaying = false;

    SetColors();

    mBaseDirectory = "Assets";
    mCurrentDirectory = "Assets";
}

Editor::~Editor()
{

}

void Editor::OnUpdate(float dt)
{
    if (!mScene)
        return;

    int width, height;
    mWindow->PollSize(width, height);

    mCamera.UpdateMatrices(std::max(mViewportSize.x, 1.0f), std::max(mViewportSize.y, 1.0f));
    if (mViewportFocused && !mScenePlaying && !mGizmoFocused)
        mCamera.Input(dt);
    if (!mScenePlaying)
        UpdateShortcuts();

    auto& cam = mCameraEntity.GetComponent<CameraComponent>();
    cam.Primary = !mScenePlaying ? 2 : 0;
    cam.Projection = mCamera.Projection();
    cam.View = mCamera.View();
    cam.Position = mCamera.Position();
}

void Editor::PostPresent()
{
    // This is beautiful shitty spaghetti code
    bool shouldWait = false;

    // Change the model if needed
    if (!mModelChange.empty()) {
        if (mSelectedEntity) {
            MeshComponent& mesh = mSelectedEntity.GetComponent<MeshComponent>();
            mesh.Init(mModelChange);
        }
        mModelChange = "";
        shouldWait = true;
    }

    // Delete the entity if needed
    if (mMarkForDeletion) {
        mScene->RemoveEntity(mSelectedEntity);
        mSelectedEntity = nullptr;
        mMarkForDeletion = false;
        shouldWait = true;
    }
    // Reload the scene if needed
    if (mMarkForStop) {
        mSelectedEntity = {};
        String pathCopy = mCurrentScenePath;
        ReloadScene(pathCopy);
        mMarkForStop = false;
        shouldWait = true;
    }
    // Delete the mesh component if needed
    if (mMarkForMeshDeletion) {
        auto& mesh = mSelectedEntity.GetComponent<MeshComponent>();
        mesh.Free();
        mSelectedEntity.RemoveComponent<MeshComponent>();
        mMarkForMeshDeletion = false;
        shouldWait = true;
    }

    if (shouldWait) {
        mRHI->Wait();
    }
    AssetManager::Purge();

    // New scene if needed
    if (mMarkForClose) {
        NewScene();
        mMarkForClose = false;
    }
    // Change skybox if needed
    if (!mSkyboxChange.empty()) {
        mScene->CookSkybox(mSkyboxChange);
        mSkyboxChange = "";
    }
    Uploader::Flush();
}

void Editor::OnPhysicsTick()
{

}

void Editor::OnImGui(const Frame& frame)
{
    if (!mScene)
        return;

    PROFILE_FUNCTION();

    BeginDockSpace();
    ProjectEditor();
    SceneEditor();
    Profiler::OnUI();
    FXVolumeEditor();
    Viewport(frame);
    LogWindow();
    HierarchyPanel();
    AssetPanel();
    EntityEditor();
    EndDockSpace();
}
