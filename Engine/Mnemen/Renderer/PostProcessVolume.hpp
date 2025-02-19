//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-15 16:11:20
//

#pragma once

#include <Core/Common.hpp>
#include <glm/glm.hpp>

struct PostProcessVolume
{
public:
    PostProcessVolume() = default;
    ~PostProcessVolume() = default;

    void Load(const String& path);
    void Save(const String& path);

    String Path;

    // BEGIN SETTINGS

    // Shadows
    bool FreezeCascades = false;
    bool VisualizeCascades = false;
    float CascadeSplitLambda = 0.95f;

    // GBuffer/Deferred
    bool VisualizeMeshlets = false;
    float DirectLight = 1.0f;
    float IndirectLight = 0.4f;

    // Skybox
    bool EnableSkybox = true;

    // Posterization
    bool EnablePosterization = false;
    float PosterizationLevels = 10.0f;

    // Color Grading
    bool EnableColorGrading = false;
    float Brightness = 1.0f;
    float Exposure = 1.0f;
    float Saturation = 1.0f;
    float Contrast = 1.0f;
    float HueShift = 1.0f;
    glm::vec4 Shadows = glm::vec4(0.5f);
    glm::vec4 Highlights = glm::vec4(0.5f);
    float Balance = 1.0f;
    float Temperature = 0.0f;
    float Tint = 0.0f;
    glm::vec4 ColorFilter = glm::vec4(1.0f);
    
    // Depth of Field
    bool EnableDOF = false;
    float FocusRange = 20.0f;
    float FocusPoint = 100.0f;
    
    // Composite
    float GammaCorrection = 2.2f;

    // Pixelization
    bool EnablePixelization = false;
    int PixelSize = 5;

    // Film Grain
    bool EnableFilmGrain = false;
    float FilmGrainAmount = 0.1f;

    // END SETTINGS
};
