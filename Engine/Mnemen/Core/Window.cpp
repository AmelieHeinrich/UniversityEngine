//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-03 12:53:32
//

#include "Window.hpp"
#include "Logger.hpp"

#include <Input/Input.hpp>
#include <imgui_impl_sdl3.h>

#include <Asset/Image.hpp>

Window::Window(int width, int height, const String& title)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_CRITICAL("Failed to initialize SDL3!");
    }

    mWindow = SDL_CreateWindow(title.c_str(), width, height, 0);
    if (!mWindow) {
        LOG_CRITICAL("Failed to create window!");
    }

    Image image;
    image.Load("Assets/Icon/icon.png");

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        image.Width, image.Height, SDL_PIXELFORMAT_RGBA32, image.Pixels.data(), image.Width * 4
    );
    if (surface) {
        SDL_SetWindowIcon(mWindow, surface);
        SDL_DestroySurface(surface);
    }

    LOG_INFO("Created window {0} ({1}, {2})", title, width, height);
}

Window::~Window()
{
    if (mWindow || mRunning) {
        SDL_DestroyWindow(mWindow);
    }
    SDL_Quit();
}

void Window::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            mRunning = false;
        }
        ImGui_ImplSDL3_ProcessEvent(&event);
        Input::Update(&event);
    }
}

void Window::PollSize(int& width, int& height)
{
    SDL_GetWindowSize(mWindow, &width, &height);
}

void* Window::GetHandle()
{
    return SDL_GetPointerProperty(SDL_GetWindowProperties(mWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}

void Window::Close()
{
    mRunning = false;
    SDL_DestroyWindow(mWindow);
}
