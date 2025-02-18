//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-12-04 01:42:33
//

#include "EditorCamera.hpp"

#include <Input/Input.hpp>
#include <imgui.h>

void Camera::UpdateMatrices(int width, int height)
{
    // Save width and height
    mWidth = width;
    mHeight = height;

    // Calculate matrices
    mView = glm::lookAt(mPosition, mPosition + mForward, glm::vec3(0.0f, 1.0f, 0.0f));
    mProjection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, CAMERA_NEAR, CAMERA_FAR);

    // Calculate vectors
    mForward.x = glm::cos(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
    mForward.y = glm::sin(glm::radians(mPitch));
    mForward.z = glm::sin(glm::radians(mYaw)) * glm::cos(glm::radians(mPitch));
    mForward = glm::normalize(mForward);

    mRight = glm::normalize(glm::cross(mForward, glm::vec3(0.0f, 1.0f, 0.0f)));
    mUp = glm::normalize(glm::cross(mRight, mForward));
}

void Camera::Input(float dt)
{
    // Keyboard input
    if (Input::IsKeyHeld(SDLK_Z))
        mPosition += mForward * dt * 3.0f;
    if (Input::IsKeyHeld(SDLK_S))
        mPosition -= mForward * dt * 3.0f;
    if (Input::IsKeyHeld(SDLK_Q))
        mPosition -= mRight * dt * 3.0f;
    if (Input::IsKeyHeld(SDLK_D))
        mPosition += mRight * dt * 3.0f;

    // Mouse input
    if (Input::IsButtonPressed(SDL_BUTTON_LEFT)) {
        glm::vec2 pos = Input::GetMouseDelta();
    
        mYaw += pos.x * 0.4f;
        mPitch -= pos.y * 0.4f;
    }
}
